//#pragma once
//#ifndef _WEBCLIENT_H__
//#define _WEBCLIENT_H__
//
//namespace System
//{
//    namespace Net
//    {
//        class TC_CFRAMEWORK_API WebClient
//        {
//        private:
//            std::string m_url;
//            std::vector<char> m_data;
//            void* m_curl_handle;
//            uint32_t m_result;
//
//        public:
//            WebClient();
//            ~WebClient();
//
//            double GetInfo(uint32_t _info);
//            void WriteDebugInfo();
//            void Query(std::string _url);
//            bool DownloadFile(std::string _url, std::string _targetFile);
//            long GetResponseCode();
//
//        public:
//            std::string GetString() { return std::string(&m_data[0]); }
//            std::vector<char> GetData() { return m_data; }
//        };
//    }
//}
//#endif //_WEBCLIENT_H__