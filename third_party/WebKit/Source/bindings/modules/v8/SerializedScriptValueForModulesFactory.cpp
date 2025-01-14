// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bindings/modules/v8/SerializedScriptValueForModulesFactory.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/SerializedScriptValue.h"
#include "bindings/modules/v8/ScriptValueSerializerForModules.h"
#include "core/dom/ExceptionCode.h"

namespace blink {

PassRefPtr<SerializedScriptValue> SerializedScriptValueForModulesFactory::create(v8::Isolate* isolate, v8::Local<v8::Value> value, Transferables* transferables, WebBlobInfoArray* blobInfo, ExceptionState& exceptionState)
{
    SerializedScriptValueWriterForModules writer;
    return SerializedScriptValueFactory::create(isolate, value, writer, transferables, blobInfo, exceptionState);
}

PassRefPtr<SerializedScriptValue> SerializedScriptValueForModulesFactory::create(v8::Isolate* isolate, const String& data)
{
    SerializedScriptValueWriterForModules writer;
    writer.writeWebCoreString(data);
    String wireData = writer.takeWireString();
    return createFromWire(wireData);
}

ScriptValueSerializer::Status SerializedScriptValueForModulesFactory::doSerialize(v8::Local<v8::Value> value, SerializedScriptValueWriter& writer, Transferables* transferables, WebBlobInfoArray* blobInfo, BlobDataHandleMap& blobDataHandles, v8::TryCatch& tryCatch, String& errorMessage, v8::Isolate* isolate)
{
    ScriptValueSerializerForModules serializer(writer, transferables, blobInfo, blobDataHandles, tryCatch, ScriptState::current(isolate));
    ScriptValueSerializer::Status status = serializer.serialize(value);
    errorMessage = serializer.errorMessage();
    return status;
}

v8::Local<v8::Value> SerializedScriptValueForModulesFactory::deserialize(String& data, BlobDataHandleMap& blobDataHandles, ArrayBufferContentsArray* arrayBufferContentsArray, ImageBitmapContentsArray* imageBitmapContentsArray, v8::Isolate* isolate, MessagePortArray* messagePorts, const WebBlobInfoArray* blobInfo)
{
    if (!data.impl())
        return v8::Null(isolate);
    static_assert(sizeof(SerializedScriptValueWriter::BufferValueType) == 2, "BufferValueType should be 2 bytes");
    data.ensure16Bit();
    // FIXME: SerializedScriptValue shouldn't use String for its underlying
    // storage. Instead, it should use SharedBuffer or Vector<uint8_t>. The
    // information stored in m_data isn't even encoded in UTF-16. Instead,
    // unicode characters are encoded as UTF-8 with two code units per UChar.
    SerializedScriptValueReaderForModules reader(reinterpret_cast<const uint8_t*>(data.impl()->characters16()), 2 * data.length(), blobInfo, blobDataHandles, ScriptState::current(isolate));
    ScriptValueDeserializerForModules deserializer(reader, messagePorts, arrayBufferContentsArray, imageBitmapContentsArray);
    return deserializer.deserialize();
}

} // namespace blink

