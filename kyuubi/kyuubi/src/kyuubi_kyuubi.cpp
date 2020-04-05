#include "stdafx.h"
#include "kyuubi_kyuubi.h"
#include "kyuubi_parser.h"
#include "kyuubi_exception.h"
#include "kyuubi_std_string_dumper.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

Kyuubi::Kyuubi()
{
	m_vm = nullptr;
}

Kyuubi::Kyuubi(ConstString templateString)
{
	createFromString(templateString);
}

Kyuubi::~Kyuubi()
{
	delete m_vm;
}

bool Kyuubi::createFromString(ConstString templateString)
{
	Parser parser;
	try
	{
		delete m_vm;
		m_vm = nullptr;
		m_vm = parser.generateVM(templateString);
		return true;
	}
	catch (KyuubiException* e)
	{
		XIYUE_LOG_ERROR("Error in template: %s", e->message().cstr());
	}

	return false;
}

bool Kyuubi::createFromFile(ConstString fileName)
{
	StringFileReader reader;
	if (!reader.readFile(fileName))
		return false;

	return createFromString(reader.getText());
}

bool Kyuubi::generateToStdString(JsonObject* data, std::wstring& output)
{
	if (m_vm == nullptr)
		return false;

	StdStringDumper dumper;
	try
	{
		m_vm->dump(data, &dumper);
		output = dumper.getString();
		return true;
	}
	catch (KyuubiException* e)
	{
		XIYUE_LOG_ERROR("Error in dump: %s", e->message().cstr());
	}

	return false;
}

bool Kyuubi::genreateToString(JsonObject* data, xiyue::ConstString& output)
{
	wstring result;
	if (!generateToStdString(data, result))
		return false;

	output = result.c_str();
	return true;
}

bool Kyuubi::generateToFile(JsonObject* data, ConstString outputFileName)
{
	wstring result;
	if (!generateToStdString(data, result))
		return false;

	StringFileWriter writer(result.size());
	if (!writer.open(outputFileName))
		return false;

	writer.writeString(result);
	writer.close();
	return true;
}
