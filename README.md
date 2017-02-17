# WestlabSyslogC++
A c++ class implemented to write into Syslog

WestLab syslog class
This has been designed based on following resources
RFC5424         : https://tools.ietf.org/html/rfc5424
syslog man page : https://linux.die.net/man[C/3/openlog
online resources: http://stackoverflow.com/questions/2638654/redirect-c-stdclog-to-syslog-on-unix

### Copy the file to /usr/include and use it simply including this library to your program

##Usage
```
std::string logName = "TestLog";
WestLabSyslogStream toSyslog (logName, WestlabSyslog::user); // You can select the logLevel
toSyslog << WestlabSyslog::debug << " "
         << __func__ << " "
         <<  __LINE__  << " "
         << __FILE__ << " "
         << std::endl;
toSyslog << WestlabSyslog::info << " Message1" << 100 <<" Message2" << 200 << std::endl;
```
