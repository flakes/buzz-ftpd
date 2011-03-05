
#ifndef _FTP_LISTENER_H
#define _FTP_LISTENER_H

class CFTPListener
{
public:
  CFTPListener(unsigned short a_port, const std::string& a_bindHost);
  ~CFTPListener();
};

#endif /* !_FTP_LISTENER_H */
