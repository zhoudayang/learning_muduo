// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: query.proto

#ifndef PROTOBUF_query_2eproto__INCLUDED
#define PROTOBUF_query_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace muduo {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_query_2eproto();
void protobuf_AssignDesc_query_2eproto();
void protobuf_ShutdownFile_query_2eproto();

class Query;
class Answer;
class Empty;

// ===================================================================

class Query : public ::google::protobuf::Message {
 public:
  Query();
  virtual ~Query();

  Query(const Query& from);

  inline Query& operator=(const Query& from) {
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
  static const Query& default_instance();

  void Swap(Query* other);

  // implements Message ----------------------------------------------

  Query* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Query& from);
  void MergeFrom(const Query& from);
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

  // required int64 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::int64 id() const;
  inline void set_id(::google::protobuf::int64 value);

  // required string questioner = 2;
  inline bool has_questioner() const;
  inline void clear_questioner();
  static const int kQuestionerFieldNumber = 2;
  inline const ::std::string& questioner() const;
  inline void set_questioner(const ::std::string& value);
  inline void set_questioner(const char* value);
  inline void set_questioner(const char* value, size_t size);
  inline ::std::string* mutable_questioner();
  inline ::std::string* release_questioner();
  inline void set_allocated_questioner(::std::string* questioner);

  // repeated string question = 3;
  inline int question_size() const;
  inline void clear_question();
  static const int kQuestionFieldNumber = 3;
  inline const ::std::string& question(int index) const;
  inline ::std::string* mutable_question(int index);
  inline void set_question(int index, const ::std::string& value);
  inline void set_question(int index, const char* value);
  inline void set_question(int index, const char* value, size_t size);
  inline ::std::string* add_question();
  inline void add_question(const ::std::string& value);
  inline void add_question(const char* value);
  inline void add_question(const char* value, size_t size);
  inline const ::google::protobuf::RepeatedPtrField< ::std::string>& question() const;
  inline ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_question();

  // @@protoc_insertion_point(class_scope:muduo.Query)
 private:
  inline void set_has_id();
  inline void clear_has_id();
  inline void set_has_questioner();
  inline void clear_has_questioner();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int64 id_;
  ::std::string* questioner_;
  ::google::protobuf::RepeatedPtrField< ::std::string> question_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(3 + 31) / 32];

  friend void  protobuf_AddDesc_query_2eproto();
  friend void protobuf_AssignDesc_query_2eproto();
  friend void protobuf_ShutdownFile_query_2eproto();

  void InitAsDefaultInstance();
  static Query* default_instance_;
};
// -------------------------------------------------------------------

class Answer : public ::google::protobuf::Message {
 public:
  Answer();
  virtual ~Answer();

  Answer(const Answer& from);

  inline Answer& operator=(const Answer& from) {
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
  static const Answer& default_instance();

  void Swap(Answer* other);

  // implements Message ----------------------------------------------

  Answer* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Answer& from);
  void MergeFrom(const Answer& from);
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

  // required int64 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::int64 id() const;
  inline void set_id(::google::protobuf::int64 value);

  // required string questioner = 2;
  inline bool has_questioner() const;
  inline void clear_questioner();
  static const int kQuestionerFieldNumber = 2;
  inline const ::std::string& questioner() const;
  inline void set_questioner(const ::std::string& value);
  inline void set_questioner(const char* value);
  inline void set_questioner(const char* value, size_t size);
  inline ::std::string* mutable_questioner();
  inline ::std::string* release_questioner();
  inline void set_allocated_questioner(::std::string* questioner);

  // required string answer = 3;
  inline bool has_answer() const;
  inline void clear_answer();
  static const int kAnswerFieldNumber = 3;
  inline const ::std::string& answer() const;
  inline void set_answer(const ::std::string& value);
  inline void set_answer(const char* value);
  inline void set_answer(const char* value, size_t size);
  inline ::std::string* mutable_answer();
  inline ::std::string* release_answer();
  inline void set_allocated_answer(::std::string* answer);

  // repeated string solution = 4;
  inline int solution_size() const;
  inline void clear_solution();
  static const int kSolutionFieldNumber = 4;
  inline const ::std::string& solution(int index) const;
  inline ::std::string* mutable_solution(int index);
  inline void set_solution(int index, const ::std::string& value);
  inline void set_solution(int index, const char* value);
  inline void set_solution(int index, const char* value, size_t size);
  inline ::std::string* add_solution();
  inline void add_solution(const ::std::string& value);
  inline void add_solution(const char* value);
  inline void add_solution(const char* value, size_t size);
  inline const ::google::protobuf::RepeatedPtrField< ::std::string>& solution() const;
  inline ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_solution();

  // @@protoc_insertion_point(class_scope:muduo.Answer)
 private:
  inline void set_has_id();
  inline void clear_has_id();
  inline void set_has_questioner();
  inline void clear_has_questioner();
  inline void set_has_answer();
  inline void clear_has_answer();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int64 id_;
  ::std::string* questioner_;
  ::std::string* answer_;
  ::google::protobuf::RepeatedPtrField< ::std::string> solution_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];

  friend void  protobuf_AddDesc_query_2eproto();
  friend void protobuf_AssignDesc_query_2eproto();
  friend void protobuf_ShutdownFile_query_2eproto();

  void InitAsDefaultInstance();
  static Answer* default_instance_;
};
// -------------------------------------------------------------------

class Empty : public ::google::protobuf::Message {
 public:
  Empty();
  virtual ~Empty();

  Empty(const Empty& from);

  inline Empty& operator=(const Empty& from) {
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
  static const Empty& default_instance();

  void Swap(Empty* other);

  // implements Message ----------------------------------------------

  Empty* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Empty& from);
  void MergeFrom(const Empty& from);
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

  // optional int32 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::int32 id() const;
  inline void set_id(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:muduo.Empty)
 private:
  inline void set_has_id();
  inline void clear_has_id();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int32 id_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_query_2eproto();
  friend void protobuf_AssignDesc_query_2eproto();
  friend void protobuf_ShutdownFile_query_2eproto();

  void InitAsDefaultInstance();
  static Empty* default_instance_;
};
// ===================================================================


// ===================================================================

// Query

// required int64 id = 1;
inline bool Query::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Query::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Query::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Query::clear_id() {
  id_ = GOOGLE_LONGLONG(0);
  clear_has_id();
}
inline ::google::protobuf::int64 Query::id() const {
  return id_;
}
inline void Query::set_id(::google::protobuf::int64 value) {
  set_has_id();
  id_ = value;
}

// required string questioner = 2;
inline bool Query::has_questioner() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Query::set_has_questioner() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Query::clear_has_questioner() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Query::clear_questioner() {
  if (questioner_ != &::google::protobuf::internal::kEmptyString) {
    questioner_->clear();
  }
  clear_has_questioner();
}
inline const ::std::string& Query::questioner() const {
  return *questioner_;
}
inline void Query::set_questioner(const ::std::string& value) {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  questioner_->assign(value);
}
inline void Query::set_questioner(const char* value) {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  questioner_->assign(value);
}
inline void Query::set_questioner(const char* value, size_t size) {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  questioner_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* Query::mutable_questioner() {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  return questioner_;
}
inline ::std::string* Query::release_questioner() {
  clear_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = questioner_;
    questioner_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void Query::set_allocated_questioner(::std::string* questioner) {
  if (questioner_ != &::google::protobuf::internal::kEmptyString) {
    delete questioner_;
  }
  if (questioner) {
    set_has_questioner();
    questioner_ = questioner;
  } else {
    clear_has_questioner();
    questioner_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// repeated string question = 3;
inline int Query::question_size() const {
  return question_.size();
}
inline void Query::clear_question() {
  question_.Clear();
}
inline const ::std::string& Query::question(int index) const {
  return question_.Get(index);
}
inline ::std::string* Query::mutable_question(int index) {
  return question_.Mutable(index);
}
inline void Query::set_question(int index, const ::std::string& value) {
  question_.Mutable(index)->assign(value);
}
inline void Query::set_question(int index, const char* value) {
  question_.Mutable(index)->assign(value);
}
inline void Query::set_question(int index, const char* value, size_t size) {
  question_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
}
inline ::std::string* Query::add_question() {
  return question_.Add();
}
inline void Query::add_question(const ::std::string& value) {
  question_.Add()->assign(value);
}
inline void Query::add_question(const char* value) {
  question_.Add()->assign(value);
}
inline void Query::add_question(const char* value, size_t size) {
  question_.Add()->assign(reinterpret_cast<const char*>(value), size);
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
Query::question() const {
  return question_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
Query::mutable_question() {
  return &question_;
}

// -------------------------------------------------------------------

// Answer

// required int64 id = 1;
inline bool Answer::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Answer::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Answer::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Answer::clear_id() {
  id_ = GOOGLE_LONGLONG(0);
  clear_has_id();
}
inline ::google::protobuf::int64 Answer::id() const {
  return id_;
}
inline void Answer::set_id(::google::protobuf::int64 value) {
  set_has_id();
  id_ = value;
}

// required string questioner = 2;
inline bool Answer::has_questioner() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Answer::set_has_questioner() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Answer::clear_has_questioner() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Answer::clear_questioner() {
  if (questioner_ != &::google::protobuf::internal::kEmptyString) {
    questioner_->clear();
  }
  clear_has_questioner();
}
inline const ::std::string& Answer::questioner() const {
  return *questioner_;
}
inline void Answer::set_questioner(const ::std::string& value) {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  questioner_->assign(value);
}
inline void Answer::set_questioner(const char* value) {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  questioner_->assign(value);
}
inline void Answer::set_questioner(const char* value, size_t size) {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  questioner_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* Answer::mutable_questioner() {
  set_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    questioner_ = new ::std::string;
  }
  return questioner_;
}
inline ::std::string* Answer::release_questioner() {
  clear_has_questioner();
  if (questioner_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = questioner_;
    questioner_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void Answer::set_allocated_questioner(::std::string* questioner) {
  if (questioner_ != &::google::protobuf::internal::kEmptyString) {
    delete questioner_;
  }
  if (questioner) {
    set_has_questioner();
    questioner_ = questioner;
  } else {
    clear_has_questioner();
    questioner_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// required string answer = 3;
inline bool Answer::has_answer() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void Answer::set_has_answer() {
  _has_bits_[0] |= 0x00000004u;
}
inline void Answer::clear_has_answer() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void Answer::clear_answer() {
  if (answer_ != &::google::protobuf::internal::kEmptyString) {
    answer_->clear();
  }
  clear_has_answer();
}
inline const ::std::string& Answer::answer() const {
  return *answer_;
}
inline void Answer::set_answer(const ::std::string& value) {
  set_has_answer();
  if (answer_ == &::google::protobuf::internal::kEmptyString) {
    answer_ = new ::std::string;
  }
  answer_->assign(value);
}
inline void Answer::set_answer(const char* value) {
  set_has_answer();
  if (answer_ == &::google::protobuf::internal::kEmptyString) {
    answer_ = new ::std::string;
  }
  answer_->assign(value);
}
inline void Answer::set_answer(const char* value, size_t size) {
  set_has_answer();
  if (answer_ == &::google::protobuf::internal::kEmptyString) {
    answer_ = new ::std::string;
  }
  answer_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* Answer::mutable_answer() {
  set_has_answer();
  if (answer_ == &::google::protobuf::internal::kEmptyString) {
    answer_ = new ::std::string;
  }
  return answer_;
}
inline ::std::string* Answer::release_answer() {
  clear_has_answer();
  if (answer_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = answer_;
    answer_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void Answer::set_allocated_answer(::std::string* answer) {
  if (answer_ != &::google::protobuf::internal::kEmptyString) {
    delete answer_;
  }
  if (answer) {
    set_has_answer();
    answer_ = answer;
  } else {
    clear_has_answer();
    answer_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// repeated string solution = 4;
inline int Answer::solution_size() const {
  return solution_.size();
}
inline void Answer::clear_solution() {
  solution_.Clear();
}
inline const ::std::string& Answer::solution(int index) const {
  return solution_.Get(index);
}
inline ::std::string* Answer::mutable_solution(int index) {
  return solution_.Mutable(index);
}
inline void Answer::set_solution(int index, const ::std::string& value) {
  solution_.Mutable(index)->assign(value);
}
inline void Answer::set_solution(int index, const char* value) {
  solution_.Mutable(index)->assign(value);
}
inline void Answer::set_solution(int index, const char* value, size_t size) {
  solution_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
}
inline ::std::string* Answer::add_solution() {
  return solution_.Add();
}
inline void Answer::add_solution(const ::std::string& value) {
  solution_.Add()->assign(value);
}
inline void Answer::add_solution(const char* value) {
  solution_.Add()->assign(value);
}
inline void Answer::add_solution(const char* value, size_t size) {
  solution_.Add()->assign(reinterpret_cast<const char*>(value), size);
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
Answer::solution() const {
  return solution_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
Answer::mutable_solution() {
  return &solution_;
}

// -------------------------------------------------------------------

// Empty

// optional int32 id = 1;
inline bool Empty::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Empty::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Empty::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Empty::clear_id() {
  id_ = 0;
  clear_has_id();
}
inline ::google::protobuf::int32 Empty::id() const {
  return id_;
}
inline void Empty::set_id(::google::protobuf::int32 value) {
  set_has_id();
  id_ = value;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace muduo

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_query_2eproto__INCLUDED
