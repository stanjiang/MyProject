// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: cs_proto/role.proto

#ifndef PROTOBUF_cs_5fproto_2frole_2eproto__INCLUDED
#define PROTOBUF_cs_5fproto_2frole_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
// @@protoc_insertion_point(includes)

namespace cspkg {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_cs_5fproto_2frole_2eproto();
void protobuf_AssignDesc_cs_5fproto_2frole_2eproto();
void protobuf_ShutdownFile_cs_5fproto_2frole_2eproto();

class AccountLoginReq;
class AccountLoginRes;

// ===================================================================

class AccountLoginReq : public ::google::protobuf::Message {
 public:
  AccountLoginReq();
  virtual ~AccountLoginReq();
  
  AccountLoginReq(const AccountLoginReq& from);
  
  inline AccountLoginReq& operator=(const AccountLoginReq& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const AccountLoginReq& default_instance();
  
  void Swap(AccountLoginReq* other);
  
  // implements Message ----------------------------------------------
  
  AccountLoginReq* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const AccountLoginReq& from);
  void MergeFrom(const AccountLoginReq& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required fixed32 account = 1;
  inline bool has_account() const;
  inline void clear_account();
  static const int kAccountFieldNumber = 1;
  inline ::google::protobuf::uint32 account() const;
  inline void set_account(::google::protobuf::uint32 value);
  
  // optional string session_key = 2;
  inline bool has_session_key() const;
  inline void clear_session_key();
  static const int kSessionKeyFieldNumber = 2;
  inline const ::std::string& session_key() const;
  inline void set_session_key(const ::std::string& value);
  inline void set_session_key(const char* value);
  inline void set_session_key(const char* value, size_t size);
  inline ::std::string* mutable_session_key();
  inline ::std::string* release_session_key();
  
  // @@protoc_insertion_point(class_scope:cspkg.AccountLoginReq)
 private:
  inline void set_has_account();
  inline void clear_has_account();
  inline void set_has_session_key();
  inline void clear_has_session_key();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  ::std::string* session_key_;
  ::google::protobuf::uint32 account_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];
  
  friend void  protobuf_AddDesc_cs_5fproto_2frole_2eproto();
  friend void protobuf_AssignDesc_cs_5fproto_2frole_2eproto();
  friend void protobuf_ShutdownFile_cs_5fproto_2frole_2eproto();
  
  void InitAsDefaultInstance();
  static AccountLoginReq* default_instance_;
};
// -------------------------------------------------------------------

class AccountLoginRes : public ::google::protobuf::Message {
 public:
  AccountLoginRes();
  virtual ~AccountLoginRes();
  
  AccountLoginRes(const AccountLoginRes& from);
  
  inline AccountLoginRes& operator=(const AccountLoginRes& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const AccountLoginRes& default_instance();
  
  void Swap(AccountLoginRes* other);
  
  // implements Message ----------------------------------------------
  
  AccountLoginRes* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const AccountLoginRes& from);
  void MergeFrom(const AccountLoginRes& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required fixed32 account = 1;
  inline bool has_account() const;
  inline void clear_account();
  static const int kAccountFieldNumber = 1;
  inline ::google::protobuf::uint32 account() const;
  inline void set_account(::google::protobuf::uint32 value);
  
  // required int32 result = 2;
  inline bool has_result() const;
  inline void clear_result();
  static const int kResultFieldNumber = 2;
  inline ::google::protobuf::int32 result() const;
  inline void set_result(::google::protobuf::int32 value);
  
  // @@protoc_insertion_point(class_scope:cspkg.AccountLoginRes)
 private:
  inline void set_has_account();
  inline void clear_has_account();
  inline void set_has_result();
  inline void clear_has_result();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  ::google::protobuf::uint32 account_;
  ::google::protobuf::int32 result_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];
  
  friend void  protobuf_AddDesc_cs_5fproto_2frole_2eproto();
  friend void protobuf_AssignDesc_cs_5fproto_2frole_2eproto();
  friend void protobuf_ShutdownFile_cs_5fproto_2frole_2eproto();
  
  void InitAsDefaultInstance();
  static AccountLoginRes* default_instance_;
};
// ===================================================================


// ===================================================================

// AccountLoginReq

// required fixed32 account = 1;
inline bool AccountLoginReq::has_account() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void AccountLoginReq::set_has_account() {
  _has_bits_[0] |= 0x00000001u;
}
inline void AccountLoginReq::clear_has_account() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void AccountLoginReq::clear_account() {
  account_ = 0u;
  clear_has_account();
}
inline ::google::protobuf::uint32 AccountLoginReq::account() const {
  return account_;
}
inline void AccountLoginReq::set_account(::google::protobuf::uint32 value) {
  set_has_account();
  account_ = value;
}

// optional string session_key = 2;
inline bool AccountLoginReq::has_session_key() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void AccountLoginReq::set_has_session_key() {
  _has_bits_[0] |= 0x00000002u;
}
inline void AccountLoginReq::clear_has_session_key() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void AccountLoginReq::clear_session_key() {
  if (session_key_ != &::google::protobuf::internal::kEmptyString) {
    session_key_->clear();
  }
  clear_has_session_key();
}
inline const ::std::string& AccountLoginReq::session_key() const {
  return *session_key_;
}
inline void AccountLoginReq::set_session_key(const ::std::string& value) {
  set_has_session_key();
  if (session_key_ == &::google::protobuf::internal::kEmptyString) {
    session_key_ = new ::std::string;
  }
  session_key_->assign(value);
}
inline void AccountLoginReq::set_session_key(const char* value) {
  set_has_session_key();
  if (session_key_ == &::google::protobuf::internal::kEmptyString) {
    session_key_ = new ::std::string;
  }
  session_key_->assign(value);
}
inline void AccountLoginReq::set_session_key(const char* value, size_t size) {
  set_has_session_key();
  if (session_key_ == &::google::protobuf::internal::kEmptyString) {
    session_key_ = new ::std::string;
  }
  session_key_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* AccountLoginReq::mutable_session_key() {
  set_has_session_key();
  if (session_key_ == &::google::protobuf::internal::kEmptyString) {
    session_key_ = new ::std::string;
  }
  return session_key_;
}
inline ::std::string* AccountLoginReq::release_session_key() {
  clear_has_session_key();
  if (session_key_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = session_key_;
    session_key_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}

// -------------------------------------------------------------------

// AccountLoginRes

// required fixed32 account = 1;
inline bool AccountLoginRes::has_account() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void AccountLoginRes::set_has_account() {
  _has_bits_[0] |= 0x00000001u;
}
inline void AccountLoginRes::clear_has_account() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void AccountLoginRes::clear_account() {
  account_ = 0u;
  clear_has_account();
}
inline ::google::protobuf::uint32 AccountLoginRes::account() const {
  return account_;
}
inline void AccountLoginRes::set_account(::google::protobuf::uint32 value) {
  set_has_account();
  account_ = value;
}

// required int32 result = 2;
inline bool AccountLoginRes::has_result() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void AccountLoginRes::set_has_result() {
  _has_bits_[0] |= 0x00000002u;
}
inline void AccountLoginRes::clear_has_result() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void AccountLoginRes::clear_result() {
  result_ = 0;
  clear_has_result();
}
inline ::google::protobuf::int32 AccountLoginRes::result() const {
  return result_;
}
inline void AccountLoginRes::set_result(::google::protobuf::int32 value) {
  set_has_result();
  result_ = value;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace cspkg

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_cs_5fproto_2frole_2eproto__INCLUDED
