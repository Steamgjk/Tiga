#pragma once
#include <vector>
#include "rrr/misc/marshal.hpp"
#include "value.h"
using mdb::Value;
using rrr::Marshal;
using std::vector;

class MultiValue {
   friend std::ostream &operator<<(std::ostream &o, const MultiValue &v);

  public:
   MultiValue() : v_(NULL), n_(0) {}

   MultiValue(const Value &v) : n_(1) {
      v_ = new Value[n_];
      v_[0] = v;
   }

   MultiValue(const vector<Value> &vs) : n_(vs.size()) {
      v_ = new Value[n_];
      for (int i = 0; i < n_; i++) {
         v_[i] = vs[i];
      }
   }

   MultiValue(int n) : n_(n) { v_ = new Value[n_]; }

   MultiValue(const MultiValue &mv) : n_(mv.n_) {
      v_ = new Value[n_];
      for (int i = 0; i < n_; i++) {
         v_[i] = mv.v_[i];
      }
   }

   MultiValue(const mdb::MultiBlob &mb)
       : n_(mb.count()), v_(new Value[mb.count()]) {
      for (int i = 0; i < mb.count(); i++) {
         Value v(std::string(mb[i].data, mb[i].len));
         v_[i] = v;
      }
   }

   inline const MultiValue &operator=(const MultiValue &mv) {
      if (&mv != this) {
         if (v_) {
            delete[] v_;
            v_ = NULL;
         }
         n_ = mv.n_;
         v_ = new Value[n_];
         for (int i = 0; i < n_; i++) {
            v_[i] = mv.v_[i];
         }
      }
      return *this;
   }

   bool operator==(const MultiValue &rhs) const {
      if (n_ == rhs.size()) {
         for (int i = 0; i < n_; i++) {
            if (v_[i] != rhs[i]) {
               return false;
            }
         }
         return true;
      } else {
         return false;
      }
   }

   ~MultiValue() {
      if (v_) {
         delete[] v_;
         v_ = NULL;
      }
   }
   int size() const { return n_; }
   Value &operator[](int idx) { return v_[idx]; }
   const Value &operator[](int idx) const { return v_[idx]; }
   int compare(const MultiValue &mv) const {
      int i = 0;
      for (i = 0; i < n_ && i < mv.n_; i++) {
         int r = v_[i].compare(mv.v_[i]);
         if (r != 0) {
            return r;
         }
      }
      if (i < n_) {
         return 1;
      } else if (i < mv.n_) {
         return -1;
      }
      return 0;
   }

  private:
   Value *v_ = NULL;
   int n_ = 0;
};

bool operator<(const MultiValue &mv1, const MultiValue &mv2);
Marshal &operator<<(Marshal &m, const MultiValue &mv);
Marshal &operator>>(Marshal &m, MultiValue &mv);

struct cell_locator {
   std::string tbl_name;
   MultiValue primary_key;
   int col_id;

   bool operator==(const cell_locator &rhs) const {
      return (tbl_name == rhs.tbl_name) && (primary_key == rhs.primary_key) &&
             (col_id == rhs.col_id);
   }

   bool operator<(const cell_locator &rhs) const {
      int i = tbl_name.compare(rhs.tbl_name);
      if (i < 0) {
         return true;
      } else if (i > 0) {
         return false;
      } else {
         if (col_id < rhs.col_id) {
            return true;
         } else if (col_id > rhs.col_id) {
            return false;
         } else {
            return primary_key.compare(rhs.primary_key) < 0;
         }
      }
   }
};

struct cell_locator_t {
   char *tbl_name;
   mdb::MultiBlob primary_key;
   int col_id;

   bool operator==(const cell_locator_t &rhs) const {
      return (tbl_name == rhs.tbl_name ||
              0 == strcmp(tbl_name, rhs.tbl_name)) &&
             (primary_key == rhs.primary_key) && (col_id == rhs.col_id);
   }

   cell_locator_t(char *_tbl_name, int n, int _col_id = 0)
       : tbl_name(_tbl_name), primary_key(n), col_id(_col_id) {}
};

struct cell_locator_t_hash {
   size_t operator()(const cell_locator_t &k) const {
      size_t ret = 0;
      ret ^= std::hash<char *>()(k.tbl_name);
      ret ^= mdb::MultiBlob::hash()(k.primary_key);
      ret ^= std::hash<int>()(k.col_id);
      return ret;
   }
};

struct multi_value_hasher {
   size_t operator()(const MultiValue &key) const {
      size_t ret = 0;
      for (int i = 0; i < key.size(); i++) {
         const Value &v = key[i];
         switch (v.get_kind()) {
            case Value::I32:
               ret ^= std::hash<int32_t>()(v.get_i32());
               break;
            case Value::I64:
               ret ^= std::hash<int64_t>()(v.get_i64());
               break;
            case Value::DOUBLE:
               ret ^= std::hash<double>()(v.get_double());
               break;
            case Value::STR:
               ret ^= std::hash<std::string>()(v.get_str());
               break;
            default:
               verify(0);
         }
      }
      return ret;
   }
};

struct cell_locator_hasher {
   size_t operator()(const cell_locator &key) const {
      size_t ret;
      ret = std::hash<std::string>()(key.tbl_name);
      ret <<= 1;
      ret ^= std::hash<int>()(key.col_id);
      ret <<= 1;

      for (int i = 0; i < key.primary_key.size(); i++) {
         const Value &v = key.primary_key[i];
         switch (v.get_kind()) {
            case Value::I32:
               ret ^= std::hash<int32_t>()(v.get_i32());
               break;
            case Value::I64:
               ret ^= std::hash<int64_t>()(v.get_i64());
               break;
            case Value::DOUBLE:
               ret ^= std::hash<double>()(v.get_double());
               break;
            case Value::STR:
               ret ^= std::hash<std::string>()(v.get_str());
               break;
            default:
               verify(0);
         }
      }

      // TODO
      return ret;
   }
};
