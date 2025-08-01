#pragma once

#include "rrr.hpp"

#include <errno.h>


namespace janus {

struct ValueTimesPair {
    rrr::i64 value;
    rrr::i64 times;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const ValueTimesPair& o) {
    m << o.value;
    m << o.times;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, ValueTimesPair& o) {
    m >> o.value;
    m >> o.times;
    return m;
}

struct TxnInfoRes {
    rrr::i32 start_txn;
    rrr::i32 total_txn;
    rrr::i32 total_try;
    rrr::i32 commit_txn;
    rrr::i32 num_exhausted;
    std::vector<double> this_latency;
    std::vector<double> last_latency;
    std::vector<double> attempt_latency;
    std::vector<double> interval_latency;
    std::vector<double> all_interval_latency;
    std::vector<rrr::i32> num_try;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const TxnInfoRes& o) {
    m << o.start_txn;
    m << o.total_txn;
    m << o.total_try;
    m << o.commit_txn;
    m << o.num_exhausted;
    m << o.this_latency;
    m << o.last_latency;
    m << o.attempt_latency;
    m << o.interval_latency;
    m << o.all_interval_latency;
    m << o.num_try;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, TxnInfoRes& o) {
    m >> o.start_txn;
    m >> o.total_txn;
    m >> o.total_try;
    m >> o.commit_txn;
    m >> o.num_exhausted;
    m >> o.this_latency;
    m >> o.last_latency;
    m >> o.attempt_latency;
    m >> o.interval_latency;
    m >> o.all_interval_latency;
    m >> o.num_try;
    return m;
}

struct ServerResponse {
    std::map<std::string, ValueTimesPair> statistics;
    double cpu_util;
    rrr::i64 r_cnt_sum;
    rrr::i64 r_cnt_num;
    rrr::i64 r_sz_sum;
    rrr::i64 r_sz_num;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const ServerResponse& o) {
    m << o.statistics;
    m << o.cpu_util;
    m << o.r_cnt_sum;
    m << o.r_cnt_num;
    m << o.r_sz_sum;
    m << o.r_sz_num;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, ServerResponse& o) {
    m >> o.statistics;
    m >> o.cpu_util;
    m >> o.r_cnt_sum;
    m >> o.r_cnt_num;
    m >> o.r_sz_sum;
    m >> o.r_sz_num;
    return m;
}

struct ClientResponse {
    std::map<rrr::i32, TxnInfoRes> txn_info;
    rrr::i64 run_sec;
    rrr::i64 run_nsec;
    rrr::i64 period_sec;
    rrr::i64 period_nsec;
    rrr::i32 is_finish;
    rrr::i64 n_asking;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const ClientResponse& o) {
    m << o.txn_info;
    m << o.run_sec;
    m << o.run_nsec;
    m << o.period_sec;
    m << o.period_nsec;
    m << o.is_finish;
    m << o.n_asking;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, ClientResponse& o) {
    m >> o.txn_info;
    m >> o.run_sec;
    m >> o.run_nsec;
    m >> o.period_sec;
    m >> o.period_nsec;
    m >> o.is_finish;
    m >> o.n_asking;
    return m;
}

struct TxDispatchRequest {
    rrr::i32 id;
    rrr::i32 tx_type;
    std::vector<Value> input;
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const TxDispatchRequest& o) {
    m << o.id;
    m << o.tx_type;
    m << o.input;
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, TxDispatchRequest& o) {
    m >> o.id;
    m >> o.tx_type;
    m >> o.input;
    return m;
}

struct TxnDispatchResponse {
};

inline rrr::Marshal& operator <<(rrr::Marshal& m, const TxnDispatchResponse& o) {
    return m;
}

inline rrr::Marshal& operator >>(rrr::Marshal& m, TxnDispatchResponse& o) {
    return m;
}

class MultiPaxosService: public rrr::Service {
public:
    enum {
        FORWARD = 0x33586f09,
        PREPARE = 0x2fe7386b,
        ACCEPT = 0x451f1afe,
        DECIDE = 0x678fa9d9,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(FORWARD, this, &MultiPaxosService::__Forward__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(PREPARE, this, &MultiPaxosService::__Prepare__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCEPT, this, &MultiPaxosService::__Accept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(DECIDE, this, &MultiPaxosService::__Decide__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(FORWARD);
        svr->unreg(PREPARE);
        svr->unreg(ACCEPT);
        svr->unreg(DECIDE);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void Forward(const MarshallDeputy& cmd, rrr::DeferredReply* defer) = 0;
    virtual void Prepare(const uint64_t& slot, const ballot_t& ballot, ballot_t* max_ballot, rrr::DeferredReply* defer) = 0;
    virtual void Accept(const uint64_t& slot, const ballot_t& ballot, const MarshallDeputy& cmd, ballot_t* max_ballot, rrr::DeferredReply* defer) = 0;
    virtual void Decide(const uint64_t& slot, const ballot_t& ballot, const MarshallDeputy& cmd, rrr::DeferredReply* defer) = 0;
private:
    void __Forward__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        MarshallDeputy* in_0 = new MarshallDeputy;
        req->m >> *in_0;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Forward(*in_0, __defer__);
    }
    void __Prepare__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        ballot_t* in_1 = new ballot_t;
        req->m >> *in_1;
        ballot_t* out_0 = new ballot_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Prepare(*in_0, *in_1, out_0, __defer__);
    }
    void __Accept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        ballot_t* in_1 = new ballot_t;
        req->m >> *in_1;
        MarshallDeputy* in_2 = new MarshallDeputy;
        req->m >> *in_2;
        ballot_t* out_0 = new ballot_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Accept(*in_0, *in_1, *in_2, out_0, __defer__);
    }
    void __Decide__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        ballot_t* in_1 = new ballot_t;
        req->m >> *in_1;
        MarshallDeputy* in_2 = new MarshallDeputy;
        req->m >> *in_2;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Decide(*in_0, *in_1, *in_2, __defer__);
    }
};

class MultiPaxosProxy {
protected:
    rrr::Client* __cl__;
public:
    MultiPaxosProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_Forward(const MarshallDeputy& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MultiPaxosService::FORWARD, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Forward(const MarshallDeputy& cmd) {
        rrr::Future* __fu__ = this->async_Forward(cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Prepare(const uint64_t& slot, const ballot_t& ballot, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MultiPaxosService::PREPARE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << slot;
            *__cl__ << ballot;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Prepare(const uint64_t& slot, const ballot_t& ballot, ballot_t* max_ballot) {
        rrr::Future* __fu__ = this->async_Prepare(slot, ballot);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *max_ballot;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Accept(const uint64_t& slot, const ballot_t& ballot, const MarshallDeputy& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MultiPaxosService::ACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << slot;
            *__cl__ << ballot;
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Accept(const uint64_t& slot, const ballot_t& ballot, const MarshallDeputy& cmd, ballot_t* max_ballot) {
        rrr::Future* __fu__ = this->async_Accept(slot, ballot, cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *max_ballot;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Decide(const uint64_t& slot, const ballot_t& ballot, const MarshallDeputy& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(MultiPaxosService::DECIDE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << slot;
            *__cl__ << ballot;
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Decide(const uint64_t& slot, const ballot_t& ballot, const MarshallDeputy& cmd) {
        rrr::Future* __fu__ = this->async_Decide(slot, ballot, cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
};

class ClassicService: public rrr::Service {
public:
    enum {
        MSGSTRING = 0x2ff0de6c,
        MSGMARSHALL = 0x48ff3951,
        DISPATCH = 0x31a46cbb,
        PREPARE = 0x3ff9af5c,
        COMMIT = 0x23f48b32,
        ABORT = 0x2a061746,
        UPGRADEEPOCH = 0x1761a376,
        TRUNCATEEPOCH = 0x1e16ec03,
        RPC_NULL = 0x3d75326c,
        TAPIRACCEPT = 0x6cd6646a,
        TAPIRFASTACCEPT = 0x373b0c11,
        TAPIRDECIDE = 0x280428a6,
        RCCDISPATCH = 0x54c8fa62,
        RCCFINISH = 0x5588bbe2,
        RCCINQUIRE = 0x33d9b435,
        RCCDISPATCHRO = 0x1fd2dd13,
        RCCINQUIREVALIDATION = 0x10e4cb5d,
        RCCNOTIFYGLOBALVALIDATION = 0x4b1d8966,
        JANUSDISPATCH = 0x5db6a23d,
        RCCCOMMIT = 0x361584bf,
        JANUSCOMMIT = 0x3db5db7d,
        JANUSCOMMITWOGRAPH = 0x61854dd7,
        JANUSINQUIRE = 0x35015411,
        RCCPREACCEPT = 0x58456ae0,
        JANUSPREACCEPT = 0x4655941d,
        JANUSPREACCEPTWOGRAPH = 0x52204b15,
        RCCACCEPT = 0x27c966c7,
        JANUSACCEPT = 0x40c2dab8,
        PREACCEPTFEBRUUS = 0x3cdd9493,
        ACCEPTFEBRUUS = 0x2627ed9b,
        COMMITFEBRUUS = 0x240c2617,
        ACCDISPATCH = 0x18dbb5db,
        ACCROTXNDISPATCH = 0x3ad2902a,
        ACCVALIDATE = 0x65707775,
        ACCFINALIZE = 0x19742c66,
        ACCSTATUSQUERY = 0x419b3ed0,
        ACCRESOLVESTATUSCOORD = 0x1d41f268,
        ACCGETRECORD = 0x5fc5ea62,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(MSGSTRING, this, &ClassicService::__MsgString__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(MSGMARSHALL, this, &ClassicService::__MsgMarshall__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(DISPATCH, this, &ClassicService::__Dispatch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(PREPARE, this, &ClassicService::__Prepare__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(COMMIT, this, &ClassicService::__Commit__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ABORT, this, &ClassicService::__Abort__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(UPGRADEEPOCH, this, &ClassicService::__UpgradeEpoch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(TRUNCATEEPOCH, this, &ClassicService::__TruncateEpoch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RPC_NULL, this, &ClassicService::__rpc_null__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(TAPIRACCEPT, this, &ClassicService::__TapirAccept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(TAPIRFASTACCEPT, this, &ClassicService::__TapirFastAccept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(TAPIRDECIDE, this, &ClassicService::__TapirDecide__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCDISPATCH, this, &ClassicService::__RccDispatch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCFINISH, this, &ClassicService::__RccFinish__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCINQUIRE, this, &ClassicService::__RccInquire__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCDISPATCHRO, this, &ClassicService::__RccDispatchRo__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCINQUIREVALIDATION, this, &ClassicService::__RccInquireValidation__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCNOTIFYGLOBALVALIDATION, this, &ClassicService::__RccNotifyGlobalValidation__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(JANUSDISPATCH, this, &ClassicService::__JanusDispatch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCCOMMIT, this, &ClassicService::__RccCommit__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(JANUSCOMMIT, this, &ClassicService::__JanusCommit__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(JANUSCOMMITWOGRAPH, this, &ClassicService::__JanusCommitWoGraph__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(JANUSINQUIRE, this, &ClassicService::__JanusInquire__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCPREACCEPT, this, &ClassicService::__RccPreAccept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(JANUSPREACCEPT, this, &ClassicService::__JanusPreAccept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(JANUSPREACCEPTWOGRAPH, this, &ClassicService::__JanusPreAcceptWoGraph__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(RCCACCEPT, this, &ClassicService::__RccAccept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(JANUSACCEPT, this, &ClassicService::__JanusAccept__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(PREACCEPTFEBRUUS, this, &ClassicService::__PreAcceptFebruus__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCEPTFEBRUUS, this, &ClassicService::__AcceptFebruus__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(COMMITFEBRUUS, this, &ClassicService::__CommitFebruus__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCDISPATCH, this, &ClassicService::__AccDispatch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCROTXNDISPATCH, this, &ClassicService::__AccRotxnDispatch__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCVALIDATE, this, &ClassicService::__AccValidate__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCFINALIZE, this, &ClassicService::__AccFinalize__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCSTATUSQUERY, this, &ClassicService::__AccStatusQuery__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCRESOLVESTATUSCOORD, this, &ClassicService::__AccResolveStatusCoord__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(ACCGETRECORD, this, &ClassicService::__AccGetRecord__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(MSGSTRING);
        svr->unreg(MSGMARSHALL);
        svr->unreg(DISPATCH);
        svr->unreg(PREPARE);
        svr->unreg(COMMIT);
        svr->unreg(ABORT);
        svr->unreg(UPGRADEEPOCH);
        svr->unreg(TRUNCATEEPOCH);
        svr->unreg(RPC_NULL);
        svr->unreg(TAPIRACCEPT);
        svr->unreg(TAPIRFASTACCEPT);
        svr->unreg(TAPIRDECIDE);
        svr->unreg(RCCDISPATCH);
        svr->unreg(RCCFINISH);
        svr->unreg(RCCINQUIRE);
        svr->unreg(RCCDISPATCHRO);
        svr->unreg(RCCINQUIREVALIDATION);
        svr->unreg(RCCNOTIFYGLOBALVALIDATION);
        svr->unreg(JANUSDISPATCH);
        svr->unreg(RCCCOMMIT);
        svr->unreg(JANUSCOMMIT);
        svr->unreg(JANUSCOMMITWOGRAPH);
        svr->unreg(JANUSINQUIRE);
        svr->unreg(RCCPREACCEPT);
        svr->unreg(JANUSPREACCEPT);
        svr->unreg(JANUSPREACCEPTWOGRAPH);
        svr->unreg(RCCACCEPT);
        svr->unreg(JANUSACCEPT);
        svr->unreg(PREACCEPTFEBRUUS);
        svr->unreg(ACCEPTFEBRUUS);
        svr->unreg(COMMITFEBRUUS);
        svr->unreg(ACCDISPATCH);
        svr->unreg(ACCROTXNDISPATCH);
        svr->unreg(ACCVALIDATE);
        svr->unreg(ACCFINALIZE);
        svr->unreg(ACCSTATUSQUERY);
        svr->unreg(ACCRESOLVESTATUSCOORD);
        svr->unreg(ACCGETRECORD);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void MsgString(const std::string& arg, std::string* ret, rrr::DeferredReply* defer) = 0;
    virtual void MsgMarshall(const MarshallDeputy& arg, MarshallDeputy* ret, rrr::DeferredReply* defer) = 0;
    virtual void Dispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, rrr::i32* res, TxnOutput* output, rrr::DeferredReply* defer) = 0;
    virtual void Prepare(const rrr::i64& tid, const std::vector<rrr::i32>& sids, rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void Commit(const rrr::i64& tid, rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void Abort(const rrr::i64& tid, rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void UpgradeEpoch(const uint32_t& curr_epoch, int32_t* res, rrr::DeferredReply* defer) = 0;
    virtual void TruncateEpoch(const uint32_t& old_epoch, rrr::DeferredReply* defer) = 0;
    virtual void rpc_null(rrr::DeferredReply* defer) = 0;
    virtual void TapirAccept(const uint64_t& cmd_id, const int64_t& ballot, const int32_t& decision, rrr::DeferredReply* defer) = 0;
    virtual void TapirFastAccept(const uint64_t& cmd_id, const std::vector<SimpleCommand>& txn_cmds, rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void TapirDecide(const uint64_t& cmd_id, const rrr::i32& commit, rrr::DeferredReply* defer) = 0;
    virtual void RccDispatch(const std::vector<SimpleCommand>& cmd, rrr::i32* res, TxnOutput* output, MarshallDeputy* md_graph, rrr::DeferredReply* defer) = 0;
    virtual void RccFinish(const cmdid_t& id, const MarshallDeputy& md_graph, std::map<uint32_t, std::map<int32_t, Value>>* outputs, rrr::DeferredReply* defer) = 0;
    virtual void RccInquire(const txnid_t& txn_id, const int32_t& rank, std::map<uint64_t, parent_set_t>*, rrr::DeferredReply* defer) = 0;
    virtual void RccDispatchRo(const SimpleCommand& cmd, std::map<rrr::i32, Value>* output, rrr::DeferredReply* defer) = 0;
    virtual void RccInquireValidation(const txid_t& tx_id, const int32_t& rank, int32_t* res, rrr::DeferredReply* defer) = 0;
    virtual void RccNotifyGlobalValidation(const txid_t& tx_id, const int32_t& rank, const int32_t& res, rrr::DeferredReply* defer) = 0;
    virtual void JanusDispatch(const std::vector<SimpleCommand>& cmd, rrr::i32* res, TxnOutput* output, MarshallDeputy* ret_graph, rrr::DeferredReply* defer) = 0;
    virtual void RccCommit(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, const parent_set_t& parents, int32_t* res, TxnOutput* output, rrr::DeferredReply* defer) = 0;
    virtual void JanusCommit(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, const MarshallDeputy& graph, int32_t* res, TxnOutput* output, rrr::DeferredReply* defer) = 0;
    virtual void JanusCommitWoGraph(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, int32_t* res, TxnOutput* output, rrr::DeferredReply* defer) = 0;
    virtual void JanusInquire(const epoch_t& epoch, const txnid_t& txn_id, MarshallDeputy* ret_graph, rrr::DeferredReply* defer) = 0;
    virtual void RccPreAccept(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, rrr::i32* res, parent_set_t* x, rrr::DeferredReply* defer) = 0;
    virtual void JanusPreAccept(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, const MarshallDeputy& graph, rrr::i32* res, MarshallDeputy* ret_graph, rrr::DeferredReply* defer) = 0;
    virtual void JanusPreAcceptWoGraph(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, rrr::i32* res, MarshallDeputy* ret_graph, rrr::DeferredReply* defer) = 0;
    virtual void RccAccept(const cmdid_t& txn_id, const rrr::i32& rank, const ballot_t& ballot, const parent_set_t& p, rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void JanusAccept(const cmdid_t& txn_id, const rrr::i32& rank, const ballot_t& ballot, const MarshallDeputy& graph, rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void PreAcceptFebruus(const txid_t& tx_id, rrr::i32* ret, uint64_t* timestamp, rrr::DeferredReply* defer) = 0;
    virtual void AcceptFebruus(const txid_t& tx_id, const ballot_t& ballot, const uint64_t& timestamp, rrr::i32* ret, rrr::DeferredReply* defer) = 0;
    virtual void CommitFebruus(const txid_t& tx_id, const uint64_t& timestamp, rrr::i32* ret, rrr::DeferredReply* defer) = 0;
    virtual void AccDispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, const uint64_t& ssid_spec, const uint8_t& is_single_shard_write_only, rrr::i32* res, uint64_t* ssid_low, uint64_t* ssid_high, uint64_t* ssid_new, TxnOutput* output, uint64_t* arrival_time, uint8_t* rotxn_okay, std::pair<parid_t, uint64_t>* new_svr_ts, rrr::DeferredReply* defer) = 0;
    virtual void AccRotxnDispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, const uint64_t& ssid_spec, const uint64_t& safe_ts, rrr::i32* res, uint64_t* ssid_low, uint64_t* ssid_high, uint64_t* ssid_new, TxnOutput* output, uint64_t* arrival_time, uint8_t* rotxn_okay, std::pair<parid_t, uint64_t>* new_svr_ts, rrr::DeferredReply* defer) = 0;
    virtual void AccValidate(const rrr::i64& tid, const uint64_t& ssid_new, int8_t* res, rrr::DeferredReply* defer) = 0;
    virtual void AccFinalize(const rrr::i64& tid, const int8_t& decision, rrr::DeferredReply* defer) = 0;
    virtual void AccStatusQuery(const rrr::i64& tid, int8_t* res, rrr::DeferredReply* defer) = 0;
    virtual void AccResolveStatusCoord(const cmdid_t& cmd_id, uint8_t* status, rrr::DeferredReply* defer) = 0;
    virtual void AccGetRecord(const cmdid_t& cmd_id, uint8_t* status, uint64_t* ssid_low, uint64_t* ssid_high, rrr::DeferredReply* defer) = 0;
private:
    void __MsgString__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        std::string* in_0 = new std::string;
        req->m >> *in_0;
        std::string* out_0 = new std::string;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->MsgString(*in_0, out_0, __defer__);
    }
    void __MsgMarshall__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        MarshallDeputy* in_0 = new MarshallDeputy;
        req->m >> *in_0;
        MarshallDeputy* out_0 = new MarshallDeputy;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->MsgMarshall(*in_0, out_0, __defer__);
    }
    void __Dispatch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint32_t* in_0 = new uint32_t;
        req->m >> *in_0;
        rrr::i64* in_1 = new rrr::i64;
        req->m >> *in_1;
        MarshallDeputy* in_2 = new MarshallDeputy;
        req->m >> *in_2;
        rrr::i32* out_0 = new rrr::i32;
        TxnOutput* out_1 = new TxnOutput;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Dispatch(*in_0, *in_1, *in_2, out_0, out_1, __defer__);
    }
    void __Prepare__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i64* in_0 = new rrr::i64;
        req->m >> *in_0;
        std::vector<rrr::i32>* in_1 = new std::vector<rrr::i32>;
        req->m >> *in_1;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Prepare(*in_0, *in_1, out_0, __defer__);
    }
    void __Commit__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i64* in_0 = new rrr::i64;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Commit(*in_0, out_0, __defer__);
    }
    void __Abort__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i64* in_0 = new rrr::i64;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->Abort(*in_0, out_0, __defer__);
    }
    void __UpgradeEpoch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint32_t* in_0 = new uint32_t;
        req->m >> *in_0;
        int32_t* out_0 = new int32_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->UpgradeEpoch(*in_0, out_0, __defer__);
    }
    void __TruncateEpoch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint32_t* in_0 = new uint32_t;
        req->m >> *in_0;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->TruncateEpoch(*in_0, __defer__);
    }
    void __rpc_null__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->rpc_null(__defer__);
    }
    void __TapirAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        int64_t* in_1 = new int64_t;
        req->m >> *in_1;
        int32_t* in_2 = new int32_t;
        req->m >> *in_2;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->TapirAccept(*in_0, *in_1, *in_2, __defer__);
    }
    void __TapirFastAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        std::vector<SimpleCommand>* in_1 = new std::vector<SimpleCommand>;
        req->m >> *in_1;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->TapirFastAccept(*in_0, *in_1, out_0, __defer__);
    }
    void __TapirDecide__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint64_t* in_0 = new uint64_t;
        req->m >> *in_0;
        rrr::i32* in_1 = new rrr::i32;
        req->m >> *in_1;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->TapirDecide(*in_0, *in_1, __defer__);
    }
    void __RccDispatch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        std::vector<SimpleCommand>* in_0 = new std::vector<SimpleCommand>;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        TxnOutput* out_1 = new TxnOutput;
        MarshallDeputy* out_2 = new MarshallDeputy;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
            *sconn << *out_2;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
            delete out_1;
            delete out_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccDispatch(*in_0, out_0, out_1, out_2, __defer__);
    }
    void __RccFinish__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        MarshallDeputy* in_1 = new MarshallDeputy;
        req->m >> *in_1;
        std::map<uint32_t, std::map<int32_t, Value>>* out_0 = new std::map<uint32_t, std::map<int32_t, Value>>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccFinish(*in_0, *in_1, out_0, __defer__);
    }
    void __RccInquire__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        txnid_t* in_0 = new txnid_t;
        req->m >> *in_0;
        int32_t* in_1 = new int32_t;
        req->m >> *in_1;
        std::map<uint64_t, parent_set_t>* out_0 = new std::map<uint64_t, parent_set_t>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccInquire(*in_0, *in_1, out_0, __defer__);
    }
    void __RccDispatchRo__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        SimpleCommand* in_0 = new SimpleCommand;
        req->m >> *in_0;
        std::map<rrr::i32, Value>* out_0 = new std::map<rrr::i32, Value>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccDispatchRo(*in_0, out_0, __defer__);
    }
    void __RccInquireValidation__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        txid_t* in_0 = new txid_t;
        req->m >> *in_0;
        int32_t* in_1 = new int32_t;
        req->m >> *in_1;
        int32_t* out_0 = new int32_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccInquireValidation(*in_0, *in_1, out_0, __defer__);
    }
    void __RccNotifyGlobalValidation__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        txid_t* in_0 = new txid_t;
        req->m >> *in_0;
        int32_t* in_1 = new int32_t;
        req->m >> *in_1;
        int32_t* in_2 = new int32_t;
        req->m >> *in_2;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccNotifyGlobalValidation(*in_0, *in_1, *in_2, __defer__);
    }
    void __JanusDispatch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        std::vector<SimpleCommand>* in_0 = new std::vector<SimpleCommand>;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        TxnOutput* out_1 = new TxnOutput;
        MarshallDeputy* out_2 = new MarshallDeputy;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
            *sconn << *out_2;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
            delete out_1;
            delete out_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->JanusDispatch(*in_0, out_0, out_1, out_2, __defer__);
    }
    void __RccCommit__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rank_t* in_1 = new rank_t;
        req->m >> *in_1;
        int32_t* in_2 = new int32_t;
        req->m >> *in_2;
        parent_set_t* in_3 = new parent_set_t;
        req->m >> *in_3;
        int32_t* out_0 = new int32_t;
        TxnOutput* out_1 = new TxnOutput;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete in_3;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccCommit(*in_0, *in_1, *in_2, *in_3, out_0, out_1, __defer__);
    }
    void __JanusCommit__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rank_t* in_1 = new rank_t;
        req->m >> *in_1;
        int32_t* in_2 = new int32_t;
        req->m >> *in_2;
        MarshallDeputy* in_3 = new MarshallDeputy;
        req->m >> *in_3;
        int32_t* out_0 = new int32_t;
        TxnOutput* out_1 = new TxnOutput;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete in_3;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->JanusCommit(*in_0, *in_1, *in_2, *in_3, out_0, out_1, __defer__);
    }
    void __JanusCommitWoGraph__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rank_t* in_1 = new rank_t;
        req->m >> *in_1;
        int32_t* in_2 = new int32_t;
        req->m >> *in_2;
        int32_t* out_0 = new int32_t;
        TxnOutput* out_1 = new TxnOutput;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->JanusCommitWoGraph(*in_0, *in_1, *in_2, out_0, out_1, __defer__);
    }
    void __JanusInquire__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        epoch_t* in_0 = new epoch_t;
        req->m >> *in_0;
        txnid_t* in_1 = new txnid_t;
        req->m >> *in_1;
        MarshallDeputy* out_0 = new MarshallDeputy;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->JanusInquire(*in_0, *in_1, out_0, __defer__);
    }
    void __RccPreAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rank_t* in_1 = new rank_t;
        req->m >> *in_1;
        std::vector<SimpleCommand>* in_2 = new std::vector<SimpleCommand>;
        req->m >> *in_2;
        rrr::i32* out_0 = new rrr::i32;
        parent_set_t* out_1 = new parent_set_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccPreAccept(*in_0, *in_1, *in_2, out_0, out_1, __defer__);
    }
    void __JanusPreAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rank_t* in_1 = new rank_t;
        req->m >> *in_1;
        std::vector<SimpleCommand>* in_2 = new std::vector<SimpleCommand>;
        req->m >> *in_2;
        MarshallDeputy* in_3 = new MarshallDeputy;
        req->m >> *in_3;
        rrr::i32* out_0 = new rrr::i32;
        MarshallDeputy* out_1 = new MarshallDeputy;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete in_3;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->JanusPreAccept(*in_0, *in_1, *in_2, *in_3, out_0, out_1, __defer__);
    }
    void __JanusPreAcceptWoGraph__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rank_t* in_1 = new rank_t;
        req->m >> *in_1;
        std::vector<SimpleCommand>* in_2 = new std::vector<SimpleCommand>;
        req->m >> *in_2;
        rrr::i32* out_0 = new rrr::i32;
        MarshallDeputy* out_1 = new MarshallDeputy;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->JanusPreAcceptWoGraph(*in_0, *in_1, *in_2, out_0, out_1, __defer__);
    }
    void __RccAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rrr::i32* in_1 = new rrr::i32;
        req->m >> *in_1;
        ballot_t* in_2 = new ballot_t;
        req->m >> *in_2;
        parent_set_t* in_3 = new parent_set_t;
        req->m >> *in_3;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete in_3;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->RccAccept(*in_0, *in_1, *in_2, *in_3, out_0, __defer__);
    }
    void __JanusAccept__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        rrr::i32* in_1 = new rrr::i32;
        req->m >> *in_1;
        ballot_t* in_2 = new ballot_t;
        req->m >> *in_2;
        MarshallDeputy* in_3 = new MarshallDeputy;
        req->m >> *in_3;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete in_3;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->JanusAccept(*in_0, *in_1, *in_2, *in_3, out_0, __defer__);
    }
    void __PreAcceptFebruus__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        txid_t* in_0 = new txid_t;
        req->m >> *in_0;
        rrr::i32* out_0 = new rrr::i32;
        uint64_t* out_1 = new uint64_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
            delete out_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->PreAcceptFebruus(*in_0, out_0, out_1, __defer__);
    }
    void __AcceptFebruus__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        txid_t* in_0 = new txid_t;
        req->m >> *in_0;
        ballot_t* in_1 = new ballot_t;
        req->m >> *in_1;
        uint64_t* in_2 = new uint64_t;
        req->m >> *in_2;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AcceptFebruus(*in_0, *in_1, *in_2, out_0, __defer__);
    }
    void __CommitFebruus__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        txid_t* in_0 = new txid_t;
        req->m >> *in_0;
        uint64_t* in_1 = new uint64_t;
        req->m >> *in_1;
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->CommitFebruus(*in_0, *in_1, out_0, __defer__);
    }
    void __AccDispatch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint32_t* in_0 = new uint32_t;
        req->m >> *in_0;
        rrr::i64* in_1 = new rrr::i64;
        req->m >> *in_1;
        MarshallDeputy* in_2 = new MarshallDeputy;
        req->m >> *in_2;
        uint64_t* in_3 = new uint64_t;
        req->m >> *in_3;
        uint8_t* in_4 = new uint8_t;
        req->m >> *in_4;
        rrr::i32* out_0 = new rrr::i32;
        uint64_t* out_1 = new uint64_t;
        uint64_t* out_2 = new uint64_t;
        uint64_t* out_3 = new uint64_t;
        TxnOutput* out_4 = new TxnOutput;
        uint64_t* out_5 = new uint64_t;
        uint8_t* out_6 = new uint8_t;
        std::pair<parid_t, uint64_t>* out_7 = new std::pair<parid_t, uint64_t>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
            *sconn << *out_2;
            *sconn << *out_3;
            *sconn << *out_4;
            *sconn << *out_5;
            *sconn << *out_6;
            *sconn << *out_7;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete in_3;
            delete in_4;
            delete out_0;
            delete out_1;
            delete out_2;
            delete out_3;
            delete out_4;
            delete out_5;
            delete out_6;
            delete out_7;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AccDispatch(*in_0, *in_1, *in_2, *in_3, *in_4, out_0, out_1, out_2, out_3, out_4, out_5, out_6, out_7, __defer__);
    }
    void __AccRotxnDispatch__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        uint32_t* in_0 = new uint32_t;
        req->m >> *in_0;
        rrr::i64* in_1 = new rrr::i64;
        req->m >> *in_1;
        MarshallDeputy* in_2 = new MarshallDeputy;
        req->m >> *in_2;
        uint64_t* in_3 = new uint64_t;
        req->m >> *in_3;
        uint64_t* in_4 = new uint64_t;
        req->m >> *in_4;
        rrr::i32* out_0 = new rrr::i32;
        uint64_t* out_1 = new uint64_t;
        uint64_t* out_2 = new uint64_t;
        uint64_t* out_3 = new uint64_t;
        TxnOutput* out_4 = new TxnOutput;
        uint64_t* out_5 = new uint64_t;
        uint8_t* out_6 = new uint8_t;
        std::pair<parid_t, uint64_t>* out_7 = new std::pair<parid_t, uint64_t>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
            *sconn << *out_2;
            *sconn << *out_3;
            *sconn << *out_4;
            *sconn << *out_5;
            *sconn << *out_6;
            *sconn << *out_7;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete in_2;
            delete in_3;
            delete in_4;
            delete out_0;
            delete out_1;
            delete out_2;
            delete out_3;
            delete out_4;
            delete out_5;
            delete out_6;
            delete out_7;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AccRotxnDispatch(*in_0, *in_1, *in_2, *in_3, *in_4, out_0, out_1, out_2, out_3, out_4, out_5, out_6, out_7, __defer__);
    }
    void __AccValidate__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i64* in_0 = new rrr::i64;
        req->m >> *in_0;
        uint64_t* in_1 = new uint64_t;
        req->m >> *in_1;
        int8_t* out_0 = new int8_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AccValidate(*in_0, *in_1, out_0, __defer__);
    }
    void __AccFinalize__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i64* in_0 = new rrr::i64;
        req->m >> *in_0;
        int8_t* in_1 = new int8_t;
        req->m >> *in_1;
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete in_1;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AccFinalize(*in_0, *in_1, __defer__);
    }
    void __AccStatusQuery__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i64* in_0 = new rrr::i64;
        req->m >> *in_0;
        int8_t* out_0 = new int8_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AccStatusQuery(*in_0, out_0, __defer__);
    }
    void __AccResolveStatusCoord__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        uint8_t* out_0 = new uint8_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AccResolveStatusCoord(*in_0, out_0, __defer__);
    }
    void __AccGetRecord__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        cmdid_t* in_0 = new cmdid_t;
        req->m >> *in_0;
        uint8_t* out_0 = new uint8_t;
        uint64_t* out_1 = new uint64_t;
        uint64_t* out_2 = new uint64_t;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
            *sconn << *out_1;
            *sconn << *out_2;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
            delete out_1;
            delete out_2;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->AccGetRecord(*in_0, out_0, out_1, out_2, __defer__);
    }
};

class ClassicProxy {
protected:
    rrr::Client* __cl__;
public:
    ClassicProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_MsgString(const std::string& arg, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::MSGSTRING, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << arg;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 MsgString(const std::string& arg, std::string* ret) {
        rrr::Future* __fu__ = this->async_MsgString(arg);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *ret;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_MsgMarshall(const MarshallDeputy& arg, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::MSGMARSHALL, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << arg;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 MsgMarshall(const MarshallDeputy& arg, MarshallDeputy* ret) {
        rrr::Future* __fu__ = this->async_MsgMarshall(arg);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *ret;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Dispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::DISPATCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << coo_id;
            *__cl__ << tid;
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Dispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, rrr::i32* res, TxnOutput* output) {
        rrr::Future* __fu__ = this->async_Dispatch(coo_id, tid, cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Prepare(const rrr::i64& tid, const std::vector<rrr::i32>& sids, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::PREPARE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tid;
            *__cl__ << sids;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Prepare(const rrr::i64& tid, const std::vector<rrr::i32>& sids, rrr::i32* res) {
        rrr::Future* __fu__ = this->async_Prepare(tid, sids);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Commit(const rrr::i64& tid, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::COMMIT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tid;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Commit(const rrr::i64& tid, rrr::i32* res) {
        rrr::Future* __fu__ = this->async_Commit(tid);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_Abort(const rrr::i64& tid, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ABORT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tid;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 Abort(const rrr::i64& tid, rrr::i32* res) {
        rrr::Future* __fu__ = this->async_Abort(tid);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_UpgradeEpoch(const uint32_t& curr_epoch, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::UPGRADEEPOCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << curr_epoch;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 UpgradeEpoch(const uint32_t& curr_epoch, int32_t* res) {
        rrr::Future* __fu__ = this->async_UpgradeEpoch(curr_epoch);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_TruncateEpoch(const uint32_t& old_epoch, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::TRUNCATEEPOCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << old_epoch;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 TruncateEpoch(const uint32_t& old_epoch) {
        rrr::Future* __fu__ = this->async_TruncateEpoch(old_epoch);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_rpc_null(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RPC_NULL, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 rpc_null() {
        rrr::Future* __fu__ = this->async_rpc_null();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_TapirAccept(const uint64_t& cmd_id, const int64_t& ballot, const int32_t& decision, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::TAPIRACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
            *__cl__ << ballot;
            *__cl__ << decision;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 TapirAccept(const uint64_t& cmd_id, const int64_t& ballot, const int32_t& decision) {
        rrr::Future* __fu__ = this->async_TapirAccept(cmd_id, ballot, decision);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_TapirFastAccept(const uint64_t& cmd_id, const std::vector<SimpleCommand>& txn_cmds, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::TAPIRFASTACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
            *__cl__ << txn_cmds;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 TapirFastAccept(const uint64_t& cmd_id, const std::vector<SimpleCommand>& txn_cmds, rrr::i32* res) {
        rrr::Future* __fu__ = this->async_TapirFastAccept(cmd_id, txn_cmds);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_TapirDecide(const uint64_t& cmd_id, const rrr::i32& commit, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::TAPIRDECIDE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
            *__cl__ << commit;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 TapirDecide(const uint64_t& cmd_id, const rrr::i32& commit) {
        rrr::Future* __fu__ = this->async_TapirDecide(cmd_id, commit);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccDispatch(const std::vector<SimpleCommand>& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCDISPATCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccDispatch(const std::vector<SimpleCommand>& cmd, rrr::i32* res, TxnOutput* output, MarshallDeputy* md_graph) {
        rrr::Future* __fu__ = this->async_RccDispatch(cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
            __fu__->get_reply() >> *md_graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccFinish(const cmdid_t& id, const MarshallDeputy& md_graph, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCFINISH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << id;
            *__cl__ << md_graph;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccFinish(const cmdid_t& id, const MarshallDeputy& md_graph, std::map<uint32_t, std::map<int32_t, Value>>* outputs) {
        rrr::Future* __fu__ = this->async_RccFinish(id, md_graph);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *outputs;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccInquire(const txnid_t& txn_id, const int32_t& rank, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCINQUIRE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
            *__cl__ << rank;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccInquire(const txnid_t& txn_id, const int32_t& rank, std::map<uint64_t, parent_set_t>* out_0) {
        rrr::Future* __fu__ = this->async_RccInquire(txn_id, rank);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *out_0;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccDispatchRo(const SimpleCommand& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCDISPATCHRO, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccDispatchRo(const SimpleCommand& cmd, std::map<rrr::i32, Value>* output) {
        rrr::Future* __fu__ = this->async_RccDispatchRo(cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *output;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccInquireValidation(const txid_t& tx_id, const int32_t& rank, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCINQUIREVALIDATION, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tx_id;
            *__cl__ << rank;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccInquireValidation(const txid_t& tx_id, const int32_t& rank, int32_t* res) {
        rrr::Future* __fu__ = this->async_RccInquireValidation(tx_id, rank);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccNotifyGlobalValidation(const txid_t& tx_id, const int32_t& rank, const int32_t& res, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCNOTIFYGLOBALVALIDATION, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tx_id;
            *__cl__ << rank;
            *__cl__ << res;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccNotifyGlobalValidation(const txid_t& tx_id, const int32_t& rank, const int32_t& res) {
        rrr::Future* __fu__ = this->async_RccNotifyGlobalValidation(tx_id, rank, res);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_JanusDispatch(const std::vector<SimpleCommand>& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::JANUSDISPATCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 JanusDispatch(const std::vector<SimpleCommand>& cmd, rrr::i32* res, TxnOutput* output, MarshallDeputy* ret_graph) {
        rrr::Future* __fu__ = this->async_JanusDispatch(cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
            __fu__->get_reply() >> *ret_graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccCommit(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, const parent_set_t& parents, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCCOMMIT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << id;
            *__cl__ << rank;
            *__cl__ << need_validation;
            *__cl__ << parents;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccCommit(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, const parent_set_t& parents, int32_t* res, TxnOutput* output) {
        rrr::Future* __fu__ = this->async_RccCommit(id, rank, need_validation, parents);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_JanusCommit(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, const MarshallDeputy& graph, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::JANUSCOMMIT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << id;
            *__cl__ << rank;
            *__cl__ << need_validation;
            *__cl__ << graph;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 JanusCommit(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, const MarshallDeputy& graph, int32_t* res, TxnOutput* output) {
        rrr::Future* __fu__ = this->async_JanusCommit(id, rank, need_validation, graph);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_JanusCommitWoGraph(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::JANUSCOMMITWOGRAPH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << id;
            *__cl__ << rank;
            *__cl__ << need_validation;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 JanusCommitWoGraph(const cmdid_t& id, const rank_t& rank, const int32_t& need_validation, int32_t* res, TxnOutput* output) {
        rrr::Future* __fu__ = this->async_JanusCommitWoGraph(id, rank, need_validation);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *output;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_JanusInquire(const epoch_t& epoch, const txnid_t& txn_id, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::JANUSINQUIRE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << epoch;
            *__cl__ << txn_id;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 JanusInquire(const epoch_t& epoch, const txnid_t& txn_id, MarshallDeputy* ret_graph) {
        rrr::Future* __fu__ = this->async_JanusInquire(epoch, txn_id);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *ret_graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccPreAccept(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCPREACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
            *__cl__ << rank;
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccPreAccept(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, rrr::i32* res, parent_set_t* x) {
        rrr::Future* __fu__ = this->async_RccPreAccept(txn_id, rank, cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *x;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_JanusPreAccept(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, const MarshallDeputy& graph, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::JANUSPREACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
            *__cl__ << rank;
            *__cl__ << cmd;
            *__cl__ << graph;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 JanusPreAccept(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, const MarshallDeputy& graph, rrr::i32* res, MarshallDeputy* ret_graph) {
        rrr::Future* __fu__ = this->async_JanusPreAccept(txn_id, rank, cmd, graph);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *ret_graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_JanusPreAcceptWoGraph(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::JANUSPREACCEPTWOGRAPH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
            *__cl__ << rank;
            *__cl__ << cmd;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 JanusPreAcceptWoGraph(const cmdid_t& txn_id, const rank_t& rank, const std::vector<SimpleCommand>& cmd, rrr::i32* res, MarshallDeputy* ret_graph) {
        rrr::Future* __fu__ = this->async_JanusPreAcceptWoGraph(txn_id, rank, cmd);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *ret_graph;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_RccAccept(const cmdid_t& txn_id, const rrr::i32& rank, const ballot_t& ballot, const parent_set_t& p, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::RCCACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
            *__cl__ << rank;
            *__cl__ << ballot;
            *__cl__ << p;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 RccAccept(const cmdid_t& txn_id, const rrr::i32& rank, const ballot_t& ballot, const parent_set_t& p, rrr::i32* res) {
        rrr::Future* __fu__ = this->async_RccAccept(txn_id, rank, ballot, p);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_JanusAccept(const cmdid_t& txn_id, const rrr::i32& rank, const ballot_t& ballot, const MarshallDeputy& graph, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::JANUSACCEPT, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << txn_id;
            *__cl__ << rank;
            *__cl__ << ballot;
            *__cl__ << graph;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 JanusAccept(const cmdid_t& txn_id, const rrr::i32& rank, const ballot_t& ballot, const MarshallDeputy& graph, rrr::i32* res) {
        rrr::Future* __fu__ = this->async_JanusAccept(txn_id, rank, ballot, graph);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_PreAcceptFebruus(const txid_t& tx_id, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::PREACCEPTFEBRUUS, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tx_id;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 PreAcceptFebruus(const txid_t& tx_id, rrr::i32* ret, uint64_t* timestamp) {
        rrr::Future* __fu__ = this->async_PreAcceptFebruus(tx_id);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *ret;
            __fu__->get_reply() >> *timestamp;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AcceptFebruus(const txid_t& tx_id, const ballot_t& ballot, const uint64_t& timestamp, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCEPTFEBRUUS, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tx_id;
            *__cl__ << ballot;
            *__cl__ << timestamp;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AcceptFebruus(const txid_t& tx_id, const ballot_t& ballot, const uint64_t& timestamp, rrr::i32* ret) {
        rrr::Future* __fu__ = this->async_AcceptFebruus(tx_id, ballot, timestamp);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *ret;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_CommitFebruus(const txid_t& tx_id, const uint64_t& timestamp, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::COMMITFEBRUUS, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tx_id;
            *__cl__ << timestamp;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 CommitFebruus(const txid_t& tx_id, const uint64_t& timestamp, rrr::i32* ret) {
        rrr::Future* __fu__ = this->async_CommitFebruus(tx_id, timestamp);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *ret;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AccDispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, const uint64_t& ssid_spec, const uint8_t& is_single_shard_write_only, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCDISPATCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << coo_id;
            *__cl__ << tid;
            *__cl__ << cmd;
            *__cl__ << ssid_spec;
            *__cl__ << is_single_shard_write_only;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AccDispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, const uint64_t& ssid_spec, const uint8_t& is_single_shard_write_only, rrr::i32* res, uint64_t* ssid_low, uint64_t* ssid_high, uint64_t* ssid_new, TxnOutput* output, uint64_t* arrival_time, uint8_t* rotxn_okay, std::pair<parid_t, uint64_t>* new_svr_ts) {
        rrr::Future* __fu__ = this->async_AccDispatch(coo_id, tid, cmd, ssid_spec, is_single_shard_write_only);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *ssid_low;
            __fu__->get_reply() >> *ssid_high;
            __fu__->get_reply() >> *ssid_new;
            __fu__->get_reply() >> *output;
            __fu__->get_reply() >> *arrival_time;
            __fu__->get_reply() >> *rotxn_okay;
            __fu__->get_reply() >> *new_svr_ts;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AccRotxnDispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, const uint64_t& ssid_spec, const uint64_t& safe_ts, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCROTXNDISPATCH, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << coo_id;
            *__cl__ << tid;
            *__cl__ << cmd;
            *__cl__ << ssid_spec;
            *__cl__ << safe_ts;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AccRotxnDispatch(const uint32_t& coo_id, const rrr::i64& tid, const MarshallDeputy& cmd, const uint64_t& ssid_spec, const uint64_t& safe_ts, rrr::i32* res, uint64_t* ssid_low, uint64_t* ssid_high, uint64_t* ssid_new, TxnOutput* output, uint64_t* arrival_time, uint8_t* rotxn_okay, std::pair<parid_t, uint64_t>* new_svr_ts) {
        rrr::Future* __fu__ = this->async_AccRotxnDispatch(coo_id, tid, cmd, ssid_spec, safe_ts);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
            __fu__->get_reply() >> *ssid_low;
            __fu__->get_reply() >> *ssid_high;
            __fu__->get_reply() >> *ssid_new;
            __fu__->get_reply() >> *output;
            __fu__->get_reply() >> *arrival_time;
            __fu__->get_reply() >> *rotxn_okay;
            __fu__->get_reply() >> *new_svr_ts;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AccValidate(const rrr::i64& tid, const uint64_t& ssid_new, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCVALIDATE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tid;
            *__cl__ << ssid_new;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AccValidate(const rrr::i64& tid, const uint64_t& ssid_new, int8_t* res) {
        rrr::Future* __fu__ = this->async_AccValidate(tid, ssid_new);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AccFinalize(const rrr::i64& tid, const int8_t& decision, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCFINALIZE, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tid;
            *__cl__ << decision;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AccFinalize(const rrr::i64& tid, const int8_t& decision) {
        rrr::Future* __fu__ = this->async_AccFinalize(tid, decision);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AccStatusQuery(const rrr::i64& tid, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCSTATUSQUERY, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << tid;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AccStatusQuery(const rrr::i64& tid, int8_t* res) {
        rrr::Future* __fu__ = this->async_AccStatusQuery(tid);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AccResolveStatusCoord(const cmdid_t& cmd_id, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCRESOLVESTATUSCOORD, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AccResolveStatusCoord(const cmdid_t& cmd_id, uint8_t* status) {
        rrr::Future* __fu__ = this->async_AccResolveStatusCoord(cmd_id);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *status;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_AccGetRecord(const cmdid_t& cmd_id, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClassicService::ACCGETRECORD, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << cmd_id;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 AccGetRecord(const cmdid_t& cmd_id, uint8_t* status, uint64_t* ssid_low, uint64_t* ssid_high) {
        rrr::Future* __fu__ = this->async_AccGetRecord(cmd_id);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *status;
            __fu__->get_reply() >> *ssid_low;
            __fu__->get_reply() >> *ssid_high;
        }
        __fu__->release();
        return __ret__;
    }
};

class ServerControlService: public rrr::Service {
public:
    enum {
        SERVER_SHUTDOWN = 0x33b51172,
        SERVER_READY = 0x6d77ec06,
        SERVER_HEART_BEAT_WITH_DATA = 0x3a7607dc,
        SERVER_HEART_BEAT = 0x53517a65,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(SERVER_SHUTDOWN, this, &ServerControlService::__server_shutdown__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(SERVER_READY, this, &ServerControlService::__server_ready__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(SERVER_HEART_BEAT_WITH_DATA, this, &ServerControlService::__server_heart_beat_with_data__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(SERVER_HEART_BEAT, this, &ServerControlService::__server_heart_beat__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(SERVER_SHUTDOWN);
        svr->unreg(SERVER_READY);
        svr->unreg(SERVER_HEART_BEAT_WITH_DATA);
        svr->unreg(SERVER_HEART_BEAT);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void server_shutdown(rrr::DeferredReply* defer) = 0;
    virtual void server_ready(rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void server_heart_beat_with_data(ServerResponse* res, rrr::DeferredReply* defer) = 0;
    virtual void server_heart_beat(rrr::DeferredReply* defer) = 0;
private:
    void __server_shutdown__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->server_shutdown(__defer__);
    }
    void __server_ready__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->server_ready(out_0, __defer__);
    }
    void __server_heart_beat_with_data__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        ServerResponse* out_0 = new ServerResponse;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->server_heart_beat_with_data(out_0, __defer__);
    }
    void __server_heart_beat__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->server_heart_beat(__defer__);
    }
};

class ServerControlProxy {
protected:
    rrr::Client* __cl__;
public:
    ServerControlProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_server_shutdown(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ServerControlService::SERVER_SHUTDOWN, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 server_shutdown() {
        rrr::Future* __fu__ = this->async_server_shutdown();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_server_ready(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ServerControlService::SERVER_READY, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 server_ready(rrr::i32* res) {
        rrr::Future* __fu__ = this->async_server_ready();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_server_heart_beat_with_data(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ServerControlService::SERVER_HEART_BEAT_WITH_DATA, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 server_heart_beat_with_data(ServerResponse* res) {
        rrr::Future* __fu__ = this->async_server_heart_beat_with_data();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_server_heart_beat(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ServerControlService::SERVER_HEART_BEAT, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 server_heart_beat() {
        rrr::Future* __fu__ = this->async_server_heart_beat();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
};

class ClientControlService: public rrr::Service {
public:
    enum {
        CLIENT_GET_TXN_NAMES = 0x4e903c07,
        CLIENT_SHUTDOWN = 0x6f65941c,
        CLIENT_FORCE_STOP = 0x6ff03ab9,
        CLIENT_RESPONSE = 0x49b1b514,
        CLIENT_READY = 0x3a15dd62,
        CLIENT_READY_BLOCK = 0x54e94bd8,
        CLIENT_START = 0x5c8841d8,
        DISPATCHTXN = 0x5788dff4,
    };
    int __reg_to__(rrr::Server* svr) {
        int ret = 0;
        if ((ret = svr->reg(CLIENT_GET_TXN_NAMES, this, &ClientControlService::__client_get_txn_names__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(CLIENT_SHUTDOWN, this, &ClientControlService::__client_shutdown__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(CLIENT_FORCE_STOP, this, &ClientControlService::__client_force_stop__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(CLIENT_RESPONSE, this, &ClientControlService::__client_response__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(CLIENT_READY, this, &ClientControlService::__client_ready__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(CLIENT_READY_BLOCK, this, &ClientControlService::__client_ready_block__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(CLIENT_START, this, &ClientControlService::__client_start__wrapper__)) != 0) {
            goto err;
        }
        if ((ret = svr->reg(DISPATCHTXN, this, &ClientControlService::__DispatchTxn__wrapper__)) != 0) {
            goto err;
        }
        return 0;
    err:
        svr->unreg(CLIENT_GET_TXN_NAMES);
        svr->unreg(CLIENT_SHUTDOWN);
        svr->unreg(CLIENT_FORCE_STOP);
        svr->unreg(CLIENT_RESPONSE);
        svr->unreg(CLIENT_READY);
        svr->unreg(CLIENT_READY_BLOCK);
        svr->unreg(CLIENT_START);
        svr->unreg(DISPATCHTXN);
        return ret;
    }
    // these RPC handler functions need to be implemented by user
    // for 'raw' handlers, remember to reply req, delete req, and sconn->release(); use sconn->run_async for heavy job
    virtual void client_get_txn_names(std::map<rrr::i32, std::string>* txn_names, rrr::DeferredReply* defer) = 0;
    virtual void client_shutdown(rrr::DeferredReply* defer) = 0;
    virtual void client_force_stop(rrr::DeferredReply* defer) = 0;
    virtual void client_response(ClientResponse* res, rrr::DeferredReply* defer) = 0;
    virtual void client_ready(rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void client_ready_block(rrr::i32* res, rrr::DeferredReply* defer) = 0;
    virtual void client_start(rrr::DeferredReply* defer) = 0;
    virtual void DispatchTxn(const TxDispatchRequest& req, TxReply* result, rrr::DeferredReply* defer) = 0;
private:
    void __client_get_txn_names__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        std::map<rrr::i32, std::string>* out_0 = new std::map<rrr::i32, std::string>;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->client_get_txn_names(out_0, __defer__);
    }
    void __client_shutdown__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->client_shutdown(__defer__);
    }
    void __client_force_stop__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->client_force_stop(__defer__);
    }
    void __client_response__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        ClientResponse* out_0 = new ClientResponse;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->client_response(out_0, __defer__);
    }
    void __client_ready__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->client_ready(out_0, __defer__);
    }
    void __client_ready_block__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        rrr::i32* out_0 = new rrr::i32;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->client_ready_block(out_0, __defer__);
    }
    void __client_start__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        auto __marshal_reply__ = [=] {
        };
        auto __cleanup__ = [=] {
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->client_start(__defer__);
    }
    void __DispatchTxn__wrapper__(rrr::Request* req, rrr::ServerConnection* sconn) {
        TxDispatchRequest* in_0 = new TxDispatchRequest;
        req->m >> *in_0;
        TxReply* out_0 = new TxReply;
        auto __marshal_reply__ = [=] {
            *sconn << *out_0;
        };
        auto __cleanup__ = [=] {
            delete in_0;
            delete out_0;
        };
        rrr::DeferredReply* __defer__ = new rrr::DeferredReply(req, sconn, __marshal_reply__, __cleanup__);
        this->DispatchTxn(*in_0, out_0, __defer__);
    }
};

class ClientControlProxy {
protected:
    rrr::Client* __cl__;
public:
    ClientControlProxy(rrr::Client* cl): __cl__(cl) { }
    rrr::Future* async_client_get_txn_names(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::CLIENT_GET_TXN_NAMES, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 client_get_txn_names(std::map<rrr::i32, std::string>* txn_names) {
        rrr::Future* __fu__ = this->async_client_get_txn_names();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *txn_names;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_client_shutdown(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::CLIENT_SHUTDOWN, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 client_shutdown() {
        rrr::Future* __fu__ = this->async_client_shutdown();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_client_force_stop(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::CLIENT_FORCE_STOP, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 client_force_stop() {
        rrr::Future* __fu__ = this->async_client_force_stop();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_client_response(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::CLIENT_RESPONSE, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 client_response(ClientResponse* res) {
        rrr::Future* __fu__ = this->async_client_response();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_client_ready(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::CLIENT_READY, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 client_ready(rrr::i32* res) {
        rrr::Future* __fu__ = this->async_client_ready();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_client_ready_block(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::CLIENT_READY_BLOCK, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 client_ready_block(rrr::i32* res) {
        rrr::Future* __fu__ = this->async_client_ready_block();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *res;
        }
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_client_start(const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::CLIENT_START, __fu_attr__);
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 client_start() {
        rrr::Future* __fu__ = this->async_client_start();
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        __fu__->release();
        return __ret__;
    }
    rrr::Future* async_DispatchTxn(const TxDispatchRequest& req, const rrr::FutureAttr& __fu_attr__ = rrr::FutureAttr()) {
        rrr::Future* __fu__ = __cl__->begin_request(ClientControlService::DISPATCHTXN, __fu_attr__);
        if (__fu__ != nullptr) {
            *__cl__ << req;
        }
        __cl__->end_request();
        return __fu__;
    }
    rrr::i32 DispatchTxn(const TxDispatchRequest& req, TxReply* result) {
        rrr::Future* __fu__ = this->async_DispatchTxn(req);
        if (__fu__ == nullptr) {
            return ENOTCONN;
        }
        rrr::i32 __ret__ = __fu__->get_error_code();
        if (__ret__ == 0) {
            __fu__->get_reply() >> *result;
        }
        __fu__->release();
        return __ret__;
    }
};

} // namespace janus



