// clang-format off
#include "__dep__.h"
#include "config.h"
#include "scheduler.h"
#include "command.h"
#include "procedure.h"
#include "communicator.h"
#include "command_marshaler.h"
#include "rcc/dep_graph.h"
#include "service.h"
#include "classic/scheduler.h"
#include "tapir/scheduler.h"
#include "rcc/server.h"
#include "janus/scheduler.h"
#include "februus/scheduler.h"
#include "benchmark_control_rpc.h"
#include "access/scheduler.h"
// clang-format on

namespace janus {

ClassicServiceImpl::ClassicServiceImpl(TxLogServer* sched,
                                       rrr::PollMgr* poll_mgr,
                                       ServerControlServiceImpl* scsi)
    : scsi_(scsi), dtxn_sched_(sched) {

#ifdef PIECE_COUNT
   piece_count_timer_.start();
   piece_count_prepare_fail_ = 0;
   piece_count_prepare_success_ = 0;
#endif

   //  if (Config::GetConfig()->do_logging()) {
   //     // verify(0);  // TODO disable logging for now. JK: Enable it now
   //     auto path = Config::GetConfig()->log_path();
   //     //    recorder_ = new Recorder(path);
   //     //    poll_mgr->add(recorder_);
   //  }

   this->RegisterStats();

   if (Config::GetConfig()->do_logging()) {
      Log_info("Launch Nezha Logging Thread");
      dtxn_sched_->ConnectToNezhaRecoder();
      record_thread_ = new std::thread(&ClassicServiceImpl::NezhaLogTd, this);
   } else {
      Log_info("NO Nezha Logging Thread");
   }

   this->RegisterStats();
}

void ClassicServiceImpl::Dispatch(const uint32_t& coo_id, const i64& cmd_id,
                                  const MarshallDeputy& md, int32_t* res,
                                  TxnOutput* output,
                                  rrr::DeferredReply* defer) {
#ifdef PIECE_COUNT
   piece_count_key_t piece_count_key =
       (piece_count_key_t){header.t_type, header.p_type};
   std::map<piece_count_key_t, uint64_t>::iterator pc_it =
       piece_count_.find(piece_count_key);

   if (pc_it == piece_count_.end())
      piece_count_[piece_count_key] = 1;
   else
      piece_count_[piece_count_key]++;
   piece_count_tid_.insert(header.tid);
#endif
   shared_ptr<Marshallable> sp = md.sp_data_;
   *res = SUCCESS;
   if (!dtxn_sched()->Dispatch(cmd_id, sp, *output)) {
      *res = REJECT;
   }
   // GC earlier txns by the same coordinator
   dtxn_sched()->gc_txns(coo_id, cmd_id);
   defer->reply();
}

void ClassicServiceImpl::Prepare(const rrr::i64& tid,
                                 const std::vector<i32>& sids, rrr::i32* res,
                                 rrr::DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   const auto& func = [res, defer, tid, sids, this]() {
      auto sched = (SchedulerClassic*)dtxn_sched_;
      bool ret = sched->OnPrepare(tid, sids);
      *res = ret ? SUCCESS : REJECT;
      defer->reply();
   };
   Coroutine::CreateRun(func);
// TODO move the stat to somewhere else.
#ifdef PIECE_COUNT
   std::map<piece_count_key_t, uint64_t>::iterator pc_it;
   if (*res != SUCCESS)
      piece_count_prepare_fail_++;
   else
      piece_count_prepare_success_++;
   if (piece_count_timer_.elapsed() >= 5.0) {
      Log::info("PIECE_COUNT: txn served: %u", piece_count_tid_.size());
      Log::info("PIECE_COUNT: prepare success: %llu, failed: %llu",
                piece_count_prepare_success_, piece_count_prepare_fail_);
      for (pc_it = piece_count_.begin(); pc_it != piece_count_.end(); pc_it++)
         Log::info("PIECE_COUNT: t_type: %d, p_type: %d, started: %llu",
                   pc_it->first.t_type, pc_it->first.p_type, pc_it->second);
      piece_count_timer_.start();
   }
#endif
}

void ClassicServiceImpl::Commit(const rrr::i64& tid, rrr::i32* res,
                                rrr::DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   //  const auto& func = [tid, res, defer, this]() {
   auto sched = (SchedulerClassic*)dtxn_sched_;
   sched->OnCommit(tid, SUCCESS);
   sched->DestroyTx(tid);  // GC
   *res = SUCCESS;
   defer->reply();
   //  };
   //  Coroutine::CreateRun(func);
}

void ClassicServiceImpl::Abort(const rrr::i64& tid, rrr::i32* res,
                               rrr::DeferredReply* defer) {
   Log_debug("get abort_txn: tid: %ld", tid);
   //  std::lock_guard<std::mutex> guard(mtx_);
   //  const auto& func = [tid, res, defer, this]() {
   auto sched = (SchedulerClassic*)dtxn_sched_;
   sched->OnCommit(tid, REJECT);
   sched->DestroyTx(tid);  // GC
   *res = SUCCESS;
   defer->reply();
   //  };
   //  Coroutine::CreateRun(func);
}

void ClassicServiceImpl::rpc_null(rrr::DeferredReply* defer) { defer->reply(); }

void ClassicServiceImpl::UpgradeEpoch(const uint32_t& curr_epoch, int32_t* res,
                                      DeferredReply* defer) {
   *res = dtxn_sched()->OnUpgradeEpoch(curr_epoch);
   defer->reply();
}

void ClassicServiceImpl::TruncateEpoch(const uint32_t& old_epoch,
                                       DeferredReply* defer) {
   verify(0);
}

void ClassicServiceImpl::TapirAccept(const cmdid_t& cmd_id,
                                     const ballot_t& ballot,
                                     const int32_t& decision,
                                     rrr::DeferredReply* defer) {
   verify(0);
}

void ClassicServiceImpl::TapirFastAccept(const txid_t& tx_id,
                                         const vector<SimpleCommand>& txn_cmds,
                                         rrr::i32* res,
                                         rrr::DeferredReply* defer) {
   SchedulerTapir* sched = (SchedulerTapir*)dtxn_sched_;
   *res = sched->OnFastAccept(tx_id, txn_cmds);
   defer->reply();
}

void ClassicServiceImpl::TapirDecide(const cmdid_t& cmd_id,
                                     const rrr::i32& decision,
                                     rrr::DeferredReply* defer) {
   SchedulerTapir* sched = (SchedulerTapir*)dtxn_sched_;
   sched->OnDecide(cmd_id, decision, [defer]() { defer->reply(); });
}

void ClassicServiceImpl::RccDispatch(const vector<SimpleCommand>& cmd,
                                     int32_t* res, TxnOutput* output,
                                     MarshallDeputy* p_md_graph,
                                     DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(this->mtx_);
   RccServer* sched = (RccServer*)dtxn_sched_;
   p_md_graph->SetMarshallable(std::make_shared<RccGraph>());
   auto p = dynamic_pointer_cast<RccGraph>(p_md_graph->sp_data_);
   *res = sched->OnDispatch(cmd, output, p);
   defer->reply();
}

void ClassicServiceImpl::RccFinish(const cmdid_t& cmd_id,
                                   const MarshallDeputy& md_graph,
                                   TxnOutput* output, DeferredReply* defer) {
   const RccGraph& graph = dynamic_cast<const RccGraph&>(*md_graph.sp_data_);
   verify(graph.size() > 0);
   verify(0);
   //  std::lock_guard<std::mutex> guard(mtx_);
   RccServer* sched = (RccServer*)dtxn_sched_;
   //  sched->OnCommit(cmd_id, RANK_UNDEFINED, graph, output, [defer]() {
   //  defer->reply(); });

   stat_sz_gra_commit_.sample(graph.size());
}

void ClassicServiceImpl::RccInquire(const txnid_t& tid, const int32_t& rank,
                                    map<txid_t, parent_set_t>* ret,
                                    rrr::DeferredReply* defer) {
   //  verify(IS_MODE_RCC || IS_MODE_RO6);
   //  std::lock_guard<std::mutex> guard(mtx_);
   RccServer* p_sched = (RccServer*)dtxn_sched_;
   //  p_md_graph->SetMarshallable(std::make_shared<RccGraph>());
   //  p_sched->OnInquire(epoch,
   //                     tid,
   //                     dynamic_pointer_cast<RccGraph>(p_md_graph->sp_data_));
   p_sched->OnInquire(tid, rank, ret);
   defer->reply();
}

void ClassicServiceImpl::RccDispatchRo(const SimpleCommand& cmd,
                                       map<int32_t, Value>* output,
                                       rrr::DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   verify(0);
   auto tx = dtxn_sched_->GetOrCreateTx(cmd.root_id_, true);
   auto dtxn = dynamic_pointer_cast<RccTx>(tx);
   dtxn->start_ro(cmd, *output, defer);
}

void ClassicServiceImpl::RccInquireValidation(const txid_t& txid,
                                              const int32_t& rank, int32_t* ret,
                                              DeferredReply* defer) {
   auto* s = (RccServer*)dtxn_sched_;
   *ret = s->OnInquireValidation(txid, rank);
   defer->reply();
}

void ClassicServiceImpl::RccNotifyGlobalValidation(const txid_t& txid,
                                                   const int32_t& rank,
                                                   const int32_t& res,
                                                   DeferredReply* defer) {
   auto* s = (RccServer*)dtxn_sched_;
   s->OnNotifyGlobalValidation(txid, rank, res);
   defer->reply();
}

void ClassicServiceImpl::JanusDispatch(const vector<SimpleCommand>& cmd,
                                       int32_t* p_res, TxnOutput* p_output,
                                       MarshallDeputy* p_md_res_graph,
                                       DeferredReply* p_defer) {
   //    std::lock_guard<std::mutex> guard(this->mtx_); // TODO remove the lock.
   auto sp_graph = std::make_shared<RccGraph>();
   auto* sched = (SchedulerJanus*)dtxn_sched_;
   *p_res = sched->OnDispatch(cmd, p_output, sp_graph);
   if (sp_graph->size() <= 1) {
      p_md_res_graph->SetMarshallable(std::make_shared<EmptyGraph>());
   } else {
      p_md_res_graph->SetMarshallable(sp_graph);
   }
   verify(p_md_res_graph->kind_ != MarshallDeputy::UNKNOWN);
   p_defer->reply();
}

void ClassicServiceImpl::JanusCommit(const cmdid_t& cmd_id, const rank_t& rank,
                                     const int32_t& need_validation,
                                     const MarshallDeputy& graph, int32_t* res,
                                     TxnOutput* output, DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   verify(0);
   auto sp_graph = dynamic_pointer_cast<RccGraph>(graph.sp_data_);
   auto p_sched = (RccServer*)dtxn_sched_;
   *res = p_sched->OnCommit(cmd_id, rank, need_validation, sp_graph, output);
   defer->reply();
}

void ClassicServiceImpl::RccCommit(const cmdid_t& cmd_id, const rank_t& rank,
                                   const int32_t& need_validation,
                                   const parent_set_t& parents, int32_t* res,
                                   TxnOutput* output, DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   auto p_sched = (RccServer*)dtxn_sched_;
   *res = p_sched->OnCommit(cmd_id, rank, need_validation, parents, output);
   defer->reply();
}

void ClassicServiceImpl::JanusCommitWoGraph(const cmdid_t& cmd_id,
                                            const rank_t& rank,
                                            const int32_t& need_validation,
                                            int32_t* res, TxnOutput* output,
                                            DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   verify(0);
   auto sched = (SchedulerJanus*)dtxn_sched_;
   *res = sched->OnCommit(cmd_id, rank, need_validation, nullptr, output);
   defer->reply();
}

void ClassicServiceImpl::JanusInquire(const epoch_t& epoch, const cmdid_t& tid,
                                      MarshallDeputy* p_md_graph,
                                      rrr::DeferredReply* defer) {
   verify(0);
   //  std::lock_guard<std::mutex> guard(mtx_);
   //  p_md_graph->SetMarshallable(std::make_shared<RccGraph>());
   //  auto p_sched = (SchedulerJanus*) dtxn_sched_;
   //  p_sched->OnInquire(epoch,
   //                     tid,
   //                     dynamic_pointer_cast<RccGraph>(p_md_graph->sp_data_));
   //  defer->reply();
}

void ClassicServiceImpl::RccPreAccept(const cmdid_t& txnid, const rank_t& rank,
                                      const vector<SimpleCommand>& cmds,
                                      int32_t* res, parent_set_t* res_parents,
                                      DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   auto sched = (RccServer*)dtxn_sched_;
   *res = sched->OnPreAccept(txnid, rank, cmds, *res_parents);
   defer->reply();
}

void ClassicServiceImpl::JanusPreAccept(
    const cmdid_t& txnid, const rank_t& rank, const vector<SimpleCommand>& cmds,
    const MarshallDeputy& md_graph, int32_t* res,
    MarshallDeputy* p_md_res_graph, DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   p_md_res_graph->SetMarshallable(std::make_shared<RccGraph>());
   auto sp_graph = dynamic_pointer_cast<RccGraph>(md_graph.sp_data_);
   auto ret_sp_graph = dynamic_pointer_cast<RccGraph>(p_md_res_graph->sp_data_);
   verify(sp_graph);
   verify(ret_sp_graph);
   auto sched = (SchedulerJanus*)dtxn_sched_;
   *res = sched->OnPreAccept(txnid, rank, cmds, sp_graph, ret_sp_graph);
   defer->reply();
}

void ClassicServiceImpl::JanusPreAcceptWoGraph(
    const cmdid_t& txnid, const rank_t& rank, const vector<SimpleCommand>& cmds,
    int32_t* res, MarshallDeputy* res_graph, DeferredReply* defer) {
   //  std::lock_guard<std::mutex> guard(mtx_);
   res_graph->SetMarshallable(std::make_shared<RccGraph>());
   auto* p_sched = (SchedulerJanus*)dtxn_sched_;
   auto sp_ret_graph = dynamic_pointer_cast<RccGraph>(res_graph->sp_data_);
   *res = p_sched->OnPreAccept(txnid, rank, cmds, nullptr, sp_ret_graph);
   defer->reply();
}

void ClassicServiceImpl::RccAccept(const cmdid_t& txnid, const rank_t& rank,
                                   const ballot_t& ballot,
                                   const parent_set_t& parents, int32_t* res,
                                   DeferredReply* defer) {
   auto sched = (RccServer*)dtxn_sched_;
   *res = sched->OnAccept(txnid, rank, ballot, parents);
   defer->reply();
}

void ClassicServiceImpl::JanusAccept(const cmdid_t& txnid, const int32_t& rank,
                                     const ballot_t& ballot,
                                     const MarshallDeputy& md_graph,
                                     int32_t* res, DeferredReply* defer) {
   auto graph = dynamic_pointer_cast<RccGraph>(md_graph.sp_data_);
   verify(graph);
   verify(md_graph.kind_ == MarshallDeputy::RCC_GRAPH);
   auto sched = (SchedulerJanus*)dtxn_sched_;
   sched->OnAccept(txnid, rank, ballot, graph, res);
   defer->reply();
}

void ClassicServiceImpl::PreAcceptFebruus(const txid_t& tx_id, int32_t* res,
                                          uint64_t* timestamp,
                                          DeferredReply* defer) {
   SchedulerFebruus* sched = (SchedulerFebruus*)dtxn_sched_;
   *res = sched->OnPreAccept(tx_id, *timestamp);
   defer->reply();
}

void ClassicServiceImpl::AcceptFebruus(const txid_t& tx_id,
                                       const ballot_t& ballot,
                                       const uint64_t& timestamp, int32_t* res,
                                       DeferredReply* defer) {
   SchedulerFebruus* sched = (SchedulerFebruus*)dtxn_sched_;
   *res = sched->OnAccept(tx_id, timestamp, ballot);
   defer->reply();
}

void ClassicServiceImpl::CommitFebruus(const txid_t& tx_id,
                                       const uint64_t& timestamp, int32_t* res,
                                       DeferredReply* defer) {
   SchedulerFebruus* sched = (SchedulerFebruus*)dtxn_sched_;
   *res = sched->OnCommit(tx_id, timestamp);
   defer->reply();
}

void ClassicServiceImpl::AccRotxnDispatch(
    const uint32_t& coo_id, const i64& cmd_id, const MarshallDeputy& cmd,
    const uint64_t& ssid_spec, const uint64_t& safe_ts, int32_t* res,
    uint64_t* ssid_low, uint64_t* ssid_high, uint64_t* ssid_new,
    TxnOutput* output, uint64_t* arrival_time, uint8_t* rotxn_okay,
    std::pair<parid_t, uint64_t>* new_svr_ts, DeferredReply* defer_reply) {
   // server-side handler of AccDispatch RPC
   shared_ptr<Marshallable> sp = cmd.sp_data_;
   auto* sched = (SchedulerAcc*)dtxn_sched_;
   *res = sched->OnDispatch(cmd_id, sp, ssid_spec, safe_ts, 0, ssid_low,
                            ssid_high, ssid_new, *output, arrival_time,
                            rotxn_okay, new_svr_ts);
   // GC earlier txns by the same coordinator
   sched->gc_txns(coo_id, cmd_id);
   defer_reply->reply();
}

void ClassicServiceImpl::AccDispatch(
    const uint32_t& coo_id, const i64& cmd_id, const MarshallDeputy& md,
    const uint64_t& ssid_spec, const uint8_t& is_single_shard_write_only,
    int32_t* res, uint64_t* ssid_low, uint64_t* ssid_high, uint64_t* ssid_new,
    TxnOutput* output, uint64_t* arrival_time, uint8_t* rotxn_okay,
    std::pair<parid_t, uint64_t>* new_svr_ts, rrr::DeferredReply* defer) {
   // GJK: Log_info("AccDispatch ");
   // server-side handler of AccDispatch RPC
   shared_ptr<Marshallable> sp = md.sp_data_;
   auto* sched = (SchedulerAcc*)dtxn_sched_;
   *res = sched->OnDispatch(
       cmd_id, sp, ssid_spec, UINT64_MAX, is_single_shard_write_only, ssid_low,
       ssid_high, ssid_new, *output, arrival_time, rotxn_okay, new_svr_ts);
   // GC earlier txns by the same coordinator
   sched->gc_txns(coo_id, cmd_id);
   if (Config::GetConfig()->do_logging()) {
      // Jinkun:
      //    Delegate to another thread, asynchronous reply after persistence
      PaxosRPC::ClientRecord msg;
      msg.cmdId_ = cmd_id;
      msg.ssidHigh_ = *ssid_high;
      msg.ssidLow_ = *ssid_low;
      msg.ssidNew_ = *ssid_new;

      // auto sp_vec_piece =
      //     dynamic_pointer_cast<VecPieceData>(sp)->sp_vec_piece_data_;
      // if (sp_vec_piece) {
      //    std::set<uint32_t> keySet;
      //    // std::string keys = "";
      //    for (auto& sp_piece_data : *sp_vec_piece) {
      //       for (auto& kv : *(sp_piece_data->input.values_)) {
      //          // keys += "(" + std::to_string(kv.first) + "," +
      //          //         std::to_string(kv.second.get_i32()) + ")";
      //          keySet.insert(kv.second.get_i32());
      //       }
      //       // keys += "|||";
      //    }
      //    // Log_info("keys=%s", keys.c_str());
      //    msg.keys_.clear();
      //    for (auto& k : keySet) {
      //       msg.keys_.push_back(k);
      //    }
      // }
      // Log_info("enqueue msg");
      record_qu_.enqueue({msg, defer});
   } else {
      defer->reply();
   }
}

void ClassicServiceImpl::AccValidate(const i64& cmd_id,
                                     const uint64_t& ssid_new, int8_t* res,
                                     DeferredReply* defer) {
   // server-side handler of AccValidate RPC
   auto* sched = (SchedulerAcc*)dtxn_sched_;
   sched->OnValidate(cmd_id, ssid_new, res);
   defer->reply();
}

void ClassicServiceImpl::AccFinalize(const i64& cmd_id, const int8_t& decision,
                                     DeferredReply* defer) {
   // server-side handler of AccFinalize
   auto* sched = (SchedulerAcc*)dtxn_sched_;
   sched->OnFinalize(cmd_id, decision);
   sched->DestroyTx(cmd_id);
   defer->reply();
}

void ClassicServiceImpl::AccStatusQuery(const i64& cmd_id, int8_t* res,
                                        DeferredReply* defer) {
   verify(0);
   // server-side handler of AccStatusQuery RPC
   // this is called right after AccDispatch as these 2 RPCs are bundled
   auto* sched = (SchedulerAcc*)dtxn_sched_;
   sched->OnStatusQuery(cmd_id, res, defer);
}

void ClassicServiceImpl::AccResolveStatusCoord(const cmdid_t& cmd_id,
                                               uint8_t* status,
                                               DeferredReply* defer) {
   auto* sched = (SchedulerAcc*)dtxn_sched_;
   sched->OnResolveStatusCoord(cmd_id, status, defer);
}

void ClassicServiceImpl::AccGetRecord(const cmdid_t& cmd_id, uint8_t* status,
                                      uint64_t* ssid_low, uint64_t* ssid_high,
                                      DeferredReply* defer) {
   auto* sched = (SchedulerAcc*)dtxn_sched_;
   sched->OnGetRecord(cmd_id, status, ssid_low, ssid_high);
   defer->reply();
}

void ClassicServiceImpl::RegisterStats() {
   if (scsi_) {
      scsi_->set_recorder(recorder_);
      scsi_->set_recorder(recorder_);
      scsi_->set_stat(ServerControlServiceImpl::STAT_SZ_SCC, &stat_sz_scc_);
      scsi_->set_stat(ServerControlServiceImpl::STAT_SZ_GRAPH_START,
                      &stat_sz_gra_start_);
      scsi_->set_stat(ServerControlServiceImpl::STAT_SZ_GRAPH_COMMIT,
                      &stat_sz_gra_commit_);
      scsi_->set_stat(ServerControlServiceImpl::STAT_SZ_GRAPH_ASK,
                      &stat_sz_gra_ask_);
      scsi_->set_stat(ServerControlServiceImpl::STAT_N_ASK, &stat_n_ask_);
      scsi_->set_stat(ServerControlServiceImpl::STAT_RO6_SZ_VECTOR,
                      &stat_ro6_sz_vector_);
   }
}

void ClassicServiceImpl::NezhaLogTd() {
   std::pair<PaxosRPC::ClientRecord, rrr::DeferredReply*> eleVec[UINT8_MAX];
   verify(dtxn_sched_->nezha_proxy_ != nullptr);
   std::map<std::pair<uint64_t, uint64_t>, rrr::DeferredReply*> pending;
   uint64_t counter = 0;
   while (true) {
      int cnt = record_qu_.try_dequeue_bulk(eleVec, UINT8_MAX);

      for (uint32_t i = 0; i < cnt; i++) {
         // Log_info("Deque msg i=%u", i);
         rrr::DeferredReply* dr = eleVec[i].second;
         // dr->reply();

         // uint64_t nowTime = GetMicrosecondTimestamp();
         // nowTime += 500 * 1000u;
         // counter++;
         // pending[{nowTime, counter}] = dr;

         rrr::FutureAttr fuattr;
         std::function<void(Future*)> cb = [this, dr](Future* fu) {
            // Log_info("call back");
            // nezha::NezhaAck ack;
            // fu->get_reply() >> ack;
            // Log_info("ret = %d\n", ack.token_); // JK-TODO: Problem
            dr->reply();
         };
         fuattr.callback = cb;
         Future::safe_release(dtxn_sched_->nezha_proxy_->async_RecordRequest(
             eleVec[i].first, fuattr));
      }

      // uint64_t now = GetMicrosecondTimestamp();
      // while (!pending.empty()) {
      //    if (pending.begin()->first.first > now) {
      //       break;
      //    } else {
      //       rrr::DeferredReply* dr = pending.begin()->second;
      //       dr->reply();
      //       pending.erase(pending.begin());
      //    }
      // }
   }
}

void ClassicServiceImpl::MsgString(const string& arg, string* ret,
                                   rrr::DeferredReply* defer) {

   verify(comm_ != nullptr);
   for (auto& f : comm_->msg_string_handlers_) {
      if (f(arg, *ret)) {
         defer->reply();
         return;
      }
   }
   verify(0);
   return;
}

void ClassicServiceImpl::MsgMarshall(const MarshallDeputy& arg,
                                     MarshallDeputy* ret,
                                     rrr::DeferredReply* defer) {

   verify(comm_ != nullptr);
   for (auto& f : comm_->msg_marshall_handlers_) {
      if (f(arg, *ret)) {
         defer->reply();
         return;
      }
   }
   verify(0);
   return;
}

}  // namespace janus
