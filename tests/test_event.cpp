/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <map>
#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>

static int lastPayload = 0;
static qi::Promise<int> pPayload;
void onFire(const int& pl)
{
  pPayload.setValue(pl);
  lastPayload = pl;
}

TEST(TestObject, Simple)
{
  qi::GenericObjectBuilder ob;
  ob.advertiseEvent<void (*)(int)>("fire");
  qi::ObjectPtr obj(ob.object());
  EXPECT_EQ(1U, obj->metaObject().signalMap().size());
  int linkId = obj->connect("fire", &onFire, 0);
  obj->emitEvent("fire", 42);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(42, lastPayload);
  pPayload.reset();
  obj->emitEvent("fire", 51);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(51, lastPayload);
  pPayload.reset();
  obj->disconnect(linkId);
  obj->emitEvent("fire", 42);
  EXPECT_FALSE(pPayload.future().wait(200));
  EXPECT_EQ(51, lastPayload);
}

TEST(TestObject, ConnectBind)
{
  qi::GenericObjectBuilder ob;
  ob.advertiseEvent<void (*)(int)>("fire");
  ob.advertiseEvent<void (*)(int, int)>("fire2");
  qi::ObjectPtr obj(ob.object());
  int link = obj->connect("fire", boost::bind<void>(&onFire, _1), 0);
  obj->emitEvent("fire", 42);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(42, lastPayload);
  obj->disconnect(link);
  // The boost bind without _1 gives us a void (void) signature that does not match fire
  EXPECT_ANY_THROW({
      obj->connect("fire", boost::bind<void>(&onFire, 51)).value();
  });

  link = obj->connect("fire2", boost::bind(&onFire, _2), 0);
  EXPECT_TRUE(link != 0);
  pPayload.reset();
  obj->emitEvent("fire2", 40, 41);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(41, lastPayload);
  obj->disconnect(link);
}

TEST(TestObject, EmitMethod)
{
  lastPayload = 0;
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("fire", &onFire);
  qi::ObjectPtr obj(ob.object());
  pPayload.reset();
  obj->emitEvent("fire", 23);
  EXPECT_TRUE(pPayload.future().wait(2000));
  EXPECT_EQ(23, pPayload.future().value());
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
