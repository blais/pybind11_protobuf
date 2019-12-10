// Copyright (c) 2019 The Pybind Development Team. All rights reserved.
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "pybind11_protobuf/proto_utils.h"

#include <pybind11/cast.h>
#include <pybind11/detail/class.h>

#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>

#include "net/proto2/public/descriptor.h"
#include "net/proto2/public/message.h"
#include "pybind11/detail/common.h"

namespace pybind11 {
namespace google {

// The value of PYBIND11_PROTOBUF_MODULE_PATH will be different depending on
// whether this is being built inside or outside of google3. The value used
// inside of google3 is defined here. Outside of google3, change this value by
// passing "-DPYBIND11_PROTOBUF_MODULE_PATH=..." on the commandline.
#ifndef PYBIND11_PROTOBUF_MODULE_PATH
#define PYBIND11_PROTOBUF_MODULE_PATH google3.third_party.pybind11_protobuf
#endif

void ImportProtoModule() {
  module::import(PYBIND11_TOSTRING(PYBIND11_PROTOBUF_MODULE_PATH) ".proto");
}

std::optional<std::string> PyProtoFullName(handle py_proto) {
  if (hasattr(py_proto, "DESCRIPTOR")) {
    auto descriptor = py_proto.attr("DESCRIPTOR");
    if (hasattr(descriptor, "full_name"))
      return cast<std::string>(descriptor.attr("full_name"));
  }
  return std::nullopt;
}

bool PyProtoCheckType(handle py_proto, const std::string& expected_type) {
  if (auto optional_name = PyProtoFullName(py_proto))
    return optional_name.value() == expected_type;
  return false;
}

std::string PyProtoSerializeToString(handle py_proto) {
  if (hasattr(py_proto, "SerializeToString"))
    return cast<std::string>(py_proto.attr("SerializeToString")());
  throw std::invalid_argument("Passed python object is not a proto.");
}

template <>
std::unique_ptr<proto2::Message> PyProtoAllocateMessage(handle py_proto,
                                                        kwargs kwargs_in) {
  std::string full_type_name;
  if (isinstance<str>(py_proto)) {
    full_type_name = str(py_proto);
  } else if (auto optional_name = PyProtoFullName(py_proto)) {
    full_type_name = std::move(optional_name).value();
  } else {
    throw std::invalid_argument("Could not get the name of the proto.");
  }
  const proto2::Descriptor* descriptor =
      proto2::DescriptorPool::generated_pool()->FindMessageTypeByName(
          full_type_name);
  if (!descriptor)
    throw std::runtime_error("Proto Descriptor not found: " + full_type_name);
  const proto2::Message* prototype =
      proto2::MessageFactory::generated_factory()->GetPrototype(descriptor);
  if (!prototype)
    throw std::runtime_error(
        "Not able to generate prototype for descriptor of: " + full_type_name);
  auto message = std::unique_ptr<proto2::Message>(prototype->New());
  ProtoInitAttrs(message.get(), kwargs_in);
  return message;
}

bool AnyPackFromPyProto(handle py_proto, ::google::protobuf::Any* any_proto) {
  auto optional_name = PyProtoFullName(py_proto);
  if (!optional_name) return false;
  any_proto->set_type_url("type.googleapis.com/" + optional_name.value());
  any_proto->set_value(PyProtoSerializeToString(py_proto));
  return true;
}

// Throws an out_of_range exception if the index is bad.
void ProtoFieldContainerBase::CheckIndex(int idx, int allowed_size) const {
  if (allowed_size < 0) allowed_size = Size();
  if (idx < 0 || idx >= allowed_size)
    throw std::out_of_range(("Bad index: " + std::to_string(idx) +
                             " (max: " + std::to_string(allowed_size - 1) + ")")
                                .c_str());
}

template <>
proto2::Message* AddMessage(RepeatedFieldContainer<proto2::Message>* container,
                            kwargs kwargs_in) {
  proto2::Message* message = container->AddDefault();
  ProtoInitAttrs(message, kwargs_in);
  return message;
}

const proto2::FieldDescriptor* GetFieldDescriptor(proto2::Message* message,
                                                  const std::string& name) {
  auto* field_desc = message->GetDescriptor()->FindFieldByName(name);
  if (!field_desc) {
    std::string error = "'" + message->GetTypeName() +
                        "' object has no attribute '" + name + "'";
    PyErr_SetString(PyExc_AttributeError, error.c_str());
    throw error_already_set();
  }
  return field_desc;
}

object ProtoGetAttr(proto2::Message* message,
                    const proto2::FieldDescriptor* field_desc) {
  if (field_desc->is_map()) {
    auto* map_pair_descriptor = field_desc->message_type();
    auto* map_value_field_desc = map_pair_descriptor->FindFieldByName("value");
    auto* map_key_field_desc = map_pair_descriptor->FindFieldByName("key");
    return DispatchFieldDescriptor<GetProtoMapField>(
        map_value_field_desc, map_key_field_desc, field_desc, message);
  } else if (field_desc->is_repeated()) {
    return DispatchFieldDescriptor<GetProtoRepeatedField>(field_desc, message);
  } else {
    return DispatchFieldDescriptor<GetProtoSingularField>(field_desc, message);
  }
}

object ProtoGetAttr(proto2::Message* message, const std::string& name) {
  return ProtoGetAttr(message, GetFieldDescriptor(message, name));
}

void ProtoSetAttr(proto2::Message* message, const std::string& name,
                  handle value) {
  auto* field_desc = GetFieldDescriptor(message, name);
  if (field_desc->is_map() || field_desc->is_repeated() ||
      field_desc->type() == proto2::FieldDescriptor::TYPE_MESSAGE) {
    std::string error = "Assignment not allowed to field \"" + name +
                        "\" in protocol message object.";
    PyErr_SetString(PyExc_AttributeError, error.c_str());
    throw error_already_set();
  }
  DispatchFieldDescriptor<SetProtoSingularField>(field_desc, message, value);
}

void ProtoInitAttrs(proto2::Message* message, kwargs kwargs_in) {
  for (auto& item : kwargs_in) {
    auto* field_desc = GetFieldDescriptor(message, str(item.first));
    handle val = item.second;
    if (field_desc->is_map()) {
      // TODO(kenoslund, rwgk): Handle map fields.
    } else if (field_desc->is_repeated()) {
      DispatchFieldDescriptor<SetProtoRepeatedField>(field_desc, message, val);
    } else {
      DispatchFieldDescriptor<SetProtoSingularField>(field_desc, message, val);
    }
  }
}

std::vector<std::string> FindInitializationErrors(proto2::Message* message) {
  std::vector<std::string> errors;
  message->FindInitializationErrors(&errors);
  return errors;
}

std::vector<tuple> ListFields(proto2::Message* message) {
  std::vector<const proto2::FieldDescriptor*> fields;
  message->GetReflection()->ListFields(*message, &fields);
  std::vector<tuple> result;
  result.reserve(fields.size());
  for (auto* field_desc : fields) {
    result.push_back(
        make_tuple(cast(field_desc, return_value_policy::reference),
                   ProtoGetAttr(message, field_desc)));
  }
  return result;
}

dict EnumValuesByNumber(const proto2::EnumDescriptor* enum_descriptor) {
  dict result;
  for (int i = 0; i < enum_descriptor->value_count(); ++i) {
    auto* value_desc = enum_descriptor->value(i);
    result[cast(value_desc->number())] =
        cast(value_desc, return_value_policy::reference);
  }
  return result;
}

}  // namespace google
}  // namespace pybind11
