#pragma once

namespace kyuubi
{
	/*
		���ÿһ�� wanling �ļ�������һ�� .h ��һ�� .cpp��ֱ�Ӷ���������
		����ģ���滻��
	*/
	class KyuubiGenerator
	{
	public:
		KyuubiGenerator() = default;

	public:
		void setName(const xiyue::ConstString& name) { m_name = name; }
		void setFileName(const xiyue::ConstString& fileName) { m_fileName = fileName; }
		void setNamespace(const xiyue::ConstString& ns) { m_namespace = ns; }
		void setHHeader(const xiyue::ConstString& header) { m_hHeader = header; }
		void setHFooter(const xiyue::ConstString& footer) { m_hFooter = footer; }
		void setContentHeader(const xiyue::ConstString& header) { m_contentHeader = header; }
		void setContentFooter(const xiyue::ConstString& footer) { m_contentFooter = footer; }

	public:
		bool generate(const xiyue::ConstString& templateString, const xiyue::ConstString& targetFolder);

	private:
		xiyue::ConstString m_name;
		xiyue::ConstString m_fileName;
		xiyue::ConstString m_namespace;
		xiyue::ConstString m_hHeader;
		xiyue::ConstString m_hFooter;
		xiyue::ConstString m_contentHeader;
		xiyue::ConstString m_contentFooter;
	};
}
