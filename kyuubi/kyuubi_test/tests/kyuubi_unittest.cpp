#include "pch.h"
#include "kyuubi_kyuubi.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

TEST(KyuubiTest, basicTest)
{
	ConstString templateString = L"Hello {{ name }}! I am {{ age }} years old."_cs;
	Kyuubi kb = templateString;
	JsonObject data = JsonObject::object({ {L"name", L"tecyle"}, {L"age", 18 } });
	ConstString result;
	EXPECT_TRUE(kb.genreateToString(&data, result));
	EXPECT_CONST_STRING_EQ(result, L"Hello tecyle! I am 18 years old."_cs);
}

TEST(KyuubiTest, ifTest)
{
	Kyuubi kb = L"Hello {{ name }}, you are {{}}{% if boy %} a boy {% else %} a girl {% endif %}!"_cs;
	JsonObject data = JsonObject::object({ {L"name", L"TomCat"}, {L"boy", true} });
	ConstString result;
	EXPECT_TRUE(kb.genreateToString(&data, result));
	EXPECT_CONST_STRING_EQ(result, L"Hello TomCat, you are a boy!"_cs);

	data.setMember(L"boy"_cs, false);
	data.setMember(L"name"_cs, L"Amanda"_cs);
	EXPECT_TRUE(kb.genreateToString(&data, result));
	EXPECT_CONST_STRING_EQ(result, L"Hello Amanda, you are a girl!"_cs);
}

TEST(KyuubiTest, forTest)
{
	Kyuubi kb = L"Here is the list: {% for val in list %} '{{ val }}', {% endfor %}"_cs;
	JsonObject data = JsonObject::object({ {L"list", JsonObject::list({L"apple", L"orange", 45, 3.14})} });
	ConstString result;
	EXPECT_TRUE(kb.genreateToString(&data, result));
	EXPECT_CONST_STRING_EQ(result, L"Here is the list:'apple','orange','45','3.140000',"_cs);
}

TEST(KyuubiTest, filterTest)
{
	Kyuubi kb = L"PI = {{ pi | format_real('0.00') }}."_cs;
	JsonObject data = JsonObject::object({ {L"pi", 3.14159} });
	ConstString result;
	EXPECT_TRUE(kb.genreateToString(&data, result));
	EXPECT_CONST_STRING_EQ(result, L"PI = 3.14."_cs);
}

TEST(KyuubiTest, demoTest)
{
	Kyuubi kb = L"Hello! I am {{ name }}, and I am {{ age + \" years old\" }}.\r\n"
		L"My hobbies are:\r\n"
		L"{% for h in hobby %}\r\n"
		L"* {{ h }}{% if loop.isLastItem %}.{% else %};{% endif %}\r\n"
		L"{% endfor %}"_cs;
	JsonObject data = JsonObject::object({
		{L"name", L"Tecyle"},
		{L"age", 18},
		{L"hobby", JsonObject::list({L"sleep", L"eat", L"dreaming"})}
		});
	ConstString result;
	EXPECT_TRUE(kb.genreateToString(&data, result));
	ConstString expectedResult = L"Hello! I am Tecyle, and I am 18 years old.\r\n"
		L"My hobbies are:\r\n"
		L"* sleep;\r\n"
		L"* eat;\r\n"
		L"* dreaming.\r\n"_cs;
	EXPECT_CONST_STRING_EQ(result, expectedResult);
}
