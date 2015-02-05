#include <iostream>
#include <unistd.h>

#include "irc_bot.h"

int main()
{

    IrcBot bot("irc.freenode.net", "6667");
    //sleep(2);
    bot.conn("asasda", "guest asughs lfgl :afriaseaEvil aBafOrT");
    sleep(15);
    bot.join_channel("#agahha454");
    sleep(15);
    bot.write_data("PRIVMSG #agahha454 :YO!\r\n");
    bot.write_data("PRIVMSG #agahha454 :HOW U DOIN DUDE?\r\n");
    std::string str = "";
    while(std::getline(std::cin, str))
        if(str == "quit") break;
}

