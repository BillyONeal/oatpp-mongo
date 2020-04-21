/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *                         Benedikt-Alexander Mokroß <bam@icognize.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "oatpp-mongo/bson/StringTest.hpp"

#include "oatpp-mongo/bson/Int8Test.hpp"
#include "oatpp-mongo/bson/Int16Test.hpp"
#include "oatpp-mongo/bson/Int32Test.hpp"
#include "oatpp-mongo/bson/Int64Test.hpp"
#include "oatpp-mongo/bson/FloatTest.hpp"
#include "oatpp-mongo/bson/BooleanTest.hpp"

#include "oatpp-mongo/bson/ListTest.hpp"
#include "oatpp-mongo/bson/MapTest.hpp"
#include "oatpp-mongo/bson/ObjectTest.hpp"

#include "oatpp-test/UnitTest.hpp"

#include <iostream>


#include "./TestUtils.hpp"

#include "oatpp-mongo/driver/command/Update.hpp"
#include "oatpp-mongo/driver/command/Insert.hpp"

#include "oatpp-mongo/driver/wire/Header.hpp"
#include "oatpp-mongo/driver/wire/Message.hpp"

#include "oatpp-mongo/bson/mapping/ObjectMapper.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "oatpp-mongo/bson/Utils.hpp"

#include "oatpp/network/client/SimpleTCPConnectionProvider.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

template<class Obj>
class Setter : public oatpp::Object {

  typedef typename Obj::ObjectWrapper ObjW;

  DTO_INIT(Setter, Object)

  DTO_FIELD(ObjW, set, "$set");

};

template<class Obj>
class UpdateWrapper : public oatpp::Object {

  typedef typename Obj::ObjectWrapper ObjW;
  typedef Setter<Obj> ObjectSetter;
  typedef typename ObjectSetter::ObjectWrapper SetterW;

  DTO_INIT(UpdateWrapper, Object)

  DTO_FIELD(ObjW, q);
  DTO_FIELD(SetterW, u) = ObjectSetter::createShared();

};

class Obj1 : public oatpp::Object {

  DTO_INIT(Obj1, Object)

  DTO_FIELD(String, firstName, "first-name") = "aaa";
  DTO_FIELD(String, lastName, "last-name") = "bbbbb";
  DTO_FIELD(Int64, age) = 18;

};

class Obj2 : public oatpp::Object {

  DTO_INIT(Obj2, Object)

  DTO_FIELD(String, _id);// = "my-string-id";
  DTO_FIELD(String, hello) = "Hello-World!!!";
  DTO_FIELD(String, mongo) = "MongoDB!!!";

};

class KeyPattern : public oatpp::Object {

  DTO_INIT(KeyPattern, Object)

  DTO_FIELD(Int32, _id);

};

class WriteError : public oatpp::Object {

  DTO_INIT(WriteError, Object)

  DTO_FIELD(Int32, index);
  DTO_FIELD(Int32, code);
  DTO_FIELD(KeyPattern::ObjectWrapper, keyPattern);
  DTO_FIELD(String, errorMessage, "errmsg");

};

class Result : public oatpp::Object {

  DTO_INIT(Result, Object)

  DTO_FIELD(Int32, number, "n");
  DTO_FIELD(Float64, ok, "ok");

  DTO_FIELD(String, errorMessage, "errmsg");
  DTO_FIELD(Int32, code, "code");
  DTO_FIELD(String, codeName, "codeName");

  DTO_FIELD(List<WriteError::ObjectWrapper>::ObjectWrapper, writeErrors);

};

#include OATPP_CODEGEN_END(DTO)

typedef oatpp::data::stream::ConsistentOutputStream ConsistentOutputStream;

/*
void testWire() {

  oatpp::mongo::bson::mapping::ObjectMapper objectMapper;

  auto connectionProvider = oatpp::network::client::SimpleTCPConnectionProvider::createShared("localhost", 27017);

  auto connection = connectionProvider->getConnection();

  InsertRequest request {0, "admin.my.collection"};

  oatpp::data::stream::BufferOutputStream requestStream;
  oatpp::data::stream::BufferOutputStream requestInnerStream;

  request.writeToStream(&requestInnerStream);
  objectMapper.write(&requestInnerStream, Obj::createShared());

  auto requestData = requestInnerStream.toString();

  Header header {16 + (v_int32)requestData->getSize(), 0, 0, 2002};

  header.writeToStream(&requestStream);
  requestStream << requestData;

  auto sendData = requestStream.toString();
  auto wres = connection->writeSimple(sendData->getData(), sendData->getSize());

  OATPP_LOGD("AAA", "sent %d", wres);

  v_char8 receiveBuffer[32768];

  auto rres = connection->readSimple(receiveBuffer, 32768);

  OATPP_LOGD("AAA", "read %d", rres);

  if(rres > 0) {
    oatpp::mongo::test::TestUtils::writeBinary(receiveBuffer, rres, "result");
  }

}
*/

void testMsg() {

  oatpp::parser::json::mapping::ObjectMapper jsonMapper;
  jsonMapper.getSerializer()->getConfig()->useBeautifier = true;

  oatpp::mongo::bson::mapping::ObjectMapper commandMapper;
  commandMapper.getSerializer()->getConfig()->includeNullFields = false;

  oatpp::mongo::bson::mapping::ObjectMapper objectMapper;

  auto connectionProvider = oatpp::network::client::SimpleTCPConnectionProvider::createShared("localhost", 27017);
  auto connection = connectionProvider->getConnection();

  oatpp::data::stream::BufferOutputStream stream;

  ///////

  oatpp::mongo::driver::command::Insert insert("admin", "my.collection");

  insert.addDocument(objectMapper.writeToString(Obj2::createShared()));

  insert.writeToStream(&stream, &commandMapper, 1);

  ///////

  auto sendData = stream.toString();
  auto wres = connection->writeSimple(sendData->getData(), sendData->getSize());

  OATPP_LOGD("AAA", "sent %d", wres);

  v_char8 receiveBuffer[32768];

  auto rres = connection->readSimple(receiveBuffer, 32768);

  OATPP_LOGD("AAA", "read %d", rres);

  oatpp::mongo::test::TestUtils::writeBinary(sendData, "input");

  if(rres > 0) {
    oatpp::mongo::test::TestUtils::writeBinary(receiveBuffer, rres, "output");
  }

  {
    oatpp::parser::Caret caret(receiveBuffer, rres);
    oatpp::mongo::driver::wire::Header header;
    header.readFromCaret(caret);

    if(header.opCode != 2013) {
      return;
    }

    oatpp::mongo::driver::wire::Message response;
    response.readFromCaret(caret);
    if(caret.hasError()) {
      OATPP_LOGE("AAA", "read response error '%s'", caret.getErrorMessage());
    }

    auto section = response.sections.front();
    if(section->getType() == 0) {
      auto body = std::static_pointer_cast<oatpp::mongo::driver::wire::BodySection>(section);
      oatpp::mongo::test::TestUtils::writeBinary(body->document, "body");

      auto result = objectMapper.readFromString<Result>(body->document);
      auto json = jsonMapper.writeToString(result);

      OATPP_LOGD("AAA", "response json='\n%s\n'", json->c_str());

    }

  }

}

void runTests() {

  //testWire();
  testMsg();

//  OATPP_RUN_TEST(oatpp::mongo::test::bson::StringTest);
//
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::Int8Test);
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::Int16Test);
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::Int32Test);
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::Int64Test);
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::FloatTest);
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::BooleanTest);
//
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::ListTest);
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::MapTest);
//  OATPP_RUN_TEST(oatpp::mongo::test::bson::ObjectTest);

}

}

int main() {

  oatpp::base::Environment::init();

  runTests();

  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";

  OATPP_ASSERT(oatpp::base::Environment::getObjectsCount() == 0);

  oatpp::base::Environment::destroy();

  return 0;
}
