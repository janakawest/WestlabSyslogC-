/**
	* WestLab syslog class
	* This has been designed based on following resources
	* RFC5424 				: https://tools.ietf.org/html/rfc5424
	* syslog man page	: https://linux.die.net/man[C/3/openlog
  * online resources: http://stackoverflow.com/questions/2638654/redirect-c-stdclog-to-syslog-on-unix 
*/

/**
	* Usage 
	* std::string logName = "TestLog";
	* WestLabSyslogStream toSyslog (logName, WestlabSyslog::user); // You can select the logLevel
	*   toSyslog << WestlabSyslog::debug << " "  
  *   << __func__ << " " 
  *   <<  __LINE__  << " " 
  *   << __FILE__ << " " 
  *   << std::endl;
	*
	* toSyslog << WestlabSyslog::info << " Message1" << 100 <<" Message2" << 200 << std::endl; 
	*/

/**
	* Copy the file to /usr/include and use it simply including this library to your program
	*/

#ifndef SYSLOG_WEST
#define SYSLOG_WEST

#include <ostream>
#include <streambuf>
#include <string>
#include <syslog.h>

namespace WestlabSyslog
{
/**
	* Following is according to RFC5424.
		 Numerical         Severity
			 Code

				0       Emergency: system is unusable
				1       Alert: action must be taken immediately
				2       Critical: critical conditions
				3       Error: error conditions
				4       Warning: warning conditions
				5       Notice: normal but significant condition
				6       Informational: informational messages
				7       Debug: debug-level messages

				Table 2. Syslog Message Severities
	* **/
	enum logLevel
	{
    emergency = LOG_EMERG,
    alert     = LOG_ALERT,
    critical  = LOG_CRIT,
    error     = LOG_ERR,
    warning   = LOG_WARNING,
    notice    = LOG_NOTICE,
    info      = LOG_INFO,
    debug     = LOG_DEBUG,
	};
/**
	* Following is according to RFC5424
	  Numerical             Facility
		 Code

			0             kernel messages
			1             user-level messages
			2             mail system
			3             system daemons
			4             security/authorization messages
			5             messages generated internally by syslogd
			6             line printer subsystem
			7             network news subsystem
			8             UUCP subsystem
			9             clock daemon
		 10             security/authorization messages
		 11             FTP daemon
		 12             NTP subsystem
		 13             log audit
		 14             log alert
		 15             clock daemon (note 2)
		 16             local use 0  (local0)
		 17             local use 1  (local1)
		 18             local use 2  (local2)
		 19             local use 3  (local3)
		 20             local use 4  (local4)
		 21             local use 5  (local5)
		 22             local use 6  (local6)
		 23             local use 7  (local7)

		Table 1.  Syslog Message Facilities
	However, according to the openlog(3) - Linux man page, it supports only following types
*/	
	enum logType
	{
		auth   	= LOG_AUTH,
		authPriv=	LOG_AUTHPRIV,
		cron   	= LOG_CRON,
		log_daemon 	= LOG_DAEMON,
		ftp			=	LOG_FTP,
		kern		= LOG_KERN,
	  local0 	= LOG_LOCAL0,
    local1 	= LOG_LOCAL1,
    local2 	= LOG_LOCAL2,
    local3 	= LOG_LOCAL3,
    local4	= LOG_LOCAL4,
    local5 	= LOG_LOCAL5,
    local6 	= LOG_LOCAL6,
    local7 	= LOG_LOCAL7,
		lpr			= LOG_LPR,
    mail   	= LOG_MAIL,
    news   	= LOG_NEWS,
		wsyslog	= LOG_SYSLOG,
    user   	= LOG_USER,
    uucp   	= LOG_UUCP,
		
		/*clockd	=	LOG_CLOCK,
		ntp			=	LOG_NTP,
		logaudit=	LOG_AUDIT,
		logalert=	LOG_ALERT,
    print  	= LOG_LPR,*/
	};
}
class WestLabSyslogStream;
class WestLabSyslogStreamBuffer: public std::basic_streambuf<char>
{
	public:
		/* Prevent from doing implicit conversions*/
		explicit WestLabSyslogStreamBuffer (const std::string& name, 
																				WestlabSyslog::logType type)
																					: std::basic_streambuf<char>()
		{
			/* void openlog(const char *ident, int option, int facility);*/
			openlog(name.size () ? name.data () : nullptr, LOG_PID|LOG_CONS/*|LOG_PERROR*/, type);
		}
		~WestLabSyslogStreamBuffer () override 
		{ 
			/* Override the destructor to close the syslog
				void closelog(void);*/
			closelog(); 
		}
		
	protected:
		/* 
		 * Note that transit types were used for the convenience of mapping both 
		 * basic_string objects, and for the input/output stream objects.
		 * Moreover, the Overflow and the sync methods of basic_streambuf was used in this implementation
		*/
		int_type overflow (int_type charType = traits_type::eof()) override
    {
      if(traits_type::eq_int_type(charType, traits_type::eof()))
				sync();
			else 
				message += traits_type::to_char_type(charType);

      return charType;
    }

    int sync() override
    {
      if(message.size())
      {
				/*void syslog(int priority, const char *format, ...);*/
				syslog(lLevel, "\n %s \n", message.data());
        message.clear();
        lLevel = basiclLevel;
      }
      return 0;
    }

		/*A hack to call this function directly from WestLabSyslogStream class*/
    friend class WestLabSyslogStream;
    void SetLogLevel (WestlabSyslog::logLevel sysLogLevel) noexcept 
		{ 
			lLevel = sysLogLevel; 
		}

	private:
    WestlabSyslog::logLevel basiclLevel = WestlabSyslog::info; //-> basic/initialization log level for the messages
    WestlabSyslog::logLevel lLevel = basiclLevel; //-> log level
    std::string message; //-> The message we are writing to the syslog
};

class WestLabSyslogStream: public std::basic_ostream<char>
{
	public:
    explicit WestLabSyslogStream (const std::string& logName = std::string (), 
																	WestlabSyslog::logType lType = WestlabSyslog::user)
																		: std::basic_ostream<char>(&syslogStreamBuffer), 
																		  syslogStreamBuffer (logName, lType)
    {/*cstrctr*/}
		~ WestLabSyslogStream ()
		{/*dstrctr*/} 	

	WestLabSyslogStream& operator<<(WestlabSyslog::logLevel Llevel) noexcept
	{
		syslogStreamBuffer.SetLogLevel (Llevel);
		return (*this); 
	}
	private:
	WestLabSyslogStreamBuffer syslogStreamBuffer; //-> The syslog Stream Buffer
};
#endif // SYSLOG_WEST
