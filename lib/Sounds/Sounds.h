#include <NonBlockingRtttl.h>
#include <ArduinoJson.h>

class Sounds {

    private:
        struct songs {
            const char * rickroll2 = "Together:d=8,o=5,b=225:4d#,f.,c#.,c.6,4a#.,4g.,f.,d#.,c.,4a#,2g#,4d#,f.,c#.,c.6,2a#,g.,f.,1d#.,d#.,4f,c#.,c.6,2a#,4g.,4f,d#.,f.,g.,4g#.,g#.,4a#,c.6,4c#6,4c6,4a#,4g.,4g#,4a#,2g#";
            const char * auldlang = "AuldLang:d=4,o=6,b=125:a5,d.,8d,d,f#,e.,8d,e,8f#,8e,d.,8d,f#,a,2b.,b,a.,8f#,f#,d,e.,8d,e,8f#,8e,d.,8b5,b5,a5,2d,16p";
            const char * startrek = "Star Trek:d=4,o=5,b=63:8f.,16a#,d#.6,8d6,16a#.,16g.,16c.6,f6";
            const char * starwars = "StarWars:d=4,o=5,b=250:8a,8p,8d6,8p,8a,8p,8d6,8p,8a,8d6,8p,8a,8p,8g#,a,8a,8g#,8a,g,8f#,8g,8f#,f.,8d.,16p,p.,8a,8p,8d6,8p,8a,8p,8d6,8p,8a,8d6,8p,8a,8p,8g#,8a,8p,8g,8p,g.,8f#,8g,8p,8c6,a#,a,g";
            const char * birthday = "HappyBir:d=8,o=5,b=100:16c,16c,d,c,f,e.,16p,16c,16c,d,c,g,f.,16p,16c,16c,c6,a,f,e,d.,16p,16a#,16a#,a,f,g,f.";
            const char * rickroll = "Never Gonna:d=4,o=5,b=200:8g,8a,8c6,8a,e6,8p,e6,8p,d6.,p,8p,8g,8a,8c6,8a,d6,8p,d6,8p,c6,8b,a.,8g,8a,8c6,8a,2c6,d6,b,a,g.,8p,g,2d6,2c6.,p,8g,8a,8c6,8a,e6,8p,e6,8p,d6.,p,8p,8g,8a,8c6,8a,2g6,b,c6.,8b,a,8g,8a,8c6,8a,2c6,d6,b,a,g.,8p,g,2d6,2c6";
            const char * cinco = "Macarena:d=16,o=5,b=180:4f6,8f6,8f6,4f6,8f6,8f6,8f6,8f6,8f6,8f6,8f6,8a6,8c6,8c6,4f6,8f6,8f6,4f6,8f6,8f6,8f6,8f6,8f6,8f6,8d6,8c6,4p,4f6,8f6,8f6,4f6,8f6,8f6,8f6,8f6,8f6,8f6,8f6,8a6,4p,2c.7,4a6,8c7,8a6,8f6,4p,2p";
            const char * xmas = "WeWishYo:d=4,o=5,b=200:d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,2g,d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,1g,d,g,g,g,2f#,f#,g,f#,e,2d,a,b,8a,8a,8g,8g,d6,d,d,e,a,f#,2g";
            const char * macgyver = "MacGyver:d=8,o=5,b=160:c6,c6,c6,c6,c6,c6,c6,c6,2b,f#,4a,2g,p,c6,4c6,4b,a,b,a,4g,4e6,2a,c6,4c6,2b,p,f#,4a,2g,p,c6,4c6,4b,a,b,a,4g,4e6,2a,2b,c6,b,a,4c6,b,a,4d6,c6,b,4d6,c6,b,4e6,d6,e6,4f#6,4b,1g6";
            const char * takeonme = "TakeOnMe:d=16,o=5,b=100:8p,a#,a#,a#,8f#,8d#,8g#,8g#,g#,c6,c6,c#6,d#6,c#6,c#6,c#6,8g#,8f#,8a#,8a#,a#,g#,g#,a#,g#,a#,a#,a#,8f#,8d#,8g#,8g#,g#,c6,c6,c#6,d#6,c#6,c#6,c#6,8g#,8f#,8a#,8a#";
            const char * melody = "NokiaTun:d=4,o=5,b=225:8e6,8d6,f#,g#,8c#6,8b,d,e,8b,8a,c#,e,2a";
            const char * halloween = "Hallowee:d=4,o=5,b=160:8c6,8f,8f,8c6,8f,8f,8c6,8f,8c#6,8f,8c6,8f,8f,8c6,8f,8c6,8f,8c#6,8f,8b,8e,8e,8b,8e,8e,8b,8e,8c6,8e,8b,8e,8e,8b,8e,8e,8b,16e";
            const char * mandy = "Mandy:d=8,o=6,b=120:d#,f,d#,d,4c.,f5,d#,d,c,4d#,f,d,4c,2a#5,f5,d,4c,a#5,d.5,4c,a#5,d,4c,16g5,2f5,f5,f5,d#,d,d#,4d,c,d,4c,2a#5,f5,d,4c,a#5,d,4c,a#5,a#5,4c,2c.5,f5,f5,d#,d,d#,d.,16c,4d,c,a#5,2a#.5";
            const char * burgertime = "Burgertime:d=4,o=6,b=285:8f,8f,8f#,8f#,8g#,8g#,8a,8a,a#,f,a#,f,8g#,8c#7,8c7,8a#,8g#,8g,8g#,8g,g#,c#7,g#,c#7,g#,f7,g#,f7,g#,d#7,g#,c#7,g#,d#7,g#,c#7";
            const char * tron = "tron:d=4,o=5,b=200:8f6,8c6,8g,e,8p,8f6,8c6,8g,8f6,8c6,8g,e.";
            const char * mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
            const char * smb_under = "smb_under:d=4,o=6,b=100:32c,32p,32c7,32p,32a5,32p,32a,32p,32a#5,32p,32a#,2p";
            const char * finalcount = "FinalCou:d=4,o=5,b=125:16c#6,16b,c#6,f#,p.,16d6,16c#6,8d6,8c#6,b,p.,16d6,16c#6,d6,f#,p.,16b,16a,8b,8a,8g#,8b,a.,16c#6,16b,c#6,f#,p.,16d6,16c#6,8d6,8c#6,b,p.,16d6,16c#6,d6,f#,p.,16b,16a,8b,8a,8g#,8b,a.,16g#,16a,b.,16a,16b,8c#6,8b,8a,8g#,f#,d6,2c#.6,16c#6,16d6,16c#6,16b,1c#6,2p";
            const char * adams = "AddamsFa:d=4,o=6,b=50:32p,32c#,16f#,32a#,16f#,32c#,16c,8g#,32f#,16f,32g#,16f,32c#,16a#5,8f#,32c#,16f#,32a#,16f#,32c#,16c,8g#,32f#,16f,32c#,16d#,32f,f#,32c#,32d#,32f,f#,32c#,32d#,32g,g#,32d#,32f,32g,16g#.,32d#,32f,32g,16g#.,32c#,32d#,32f,32f#";
            const char * mspacman = "mspacman:d=4,o=5,b=100:32d,32e,8f,8a,8g,8a#,16a,16a#,16c6,16a,8g,8a#,16a,16a#,16c6,16a,16a#,16c6,16d6,16e6,8f6,8e6,8f6";
            const char * galaga = "Galaga:d=4,o=5,b=125:8g4,32c,32p,8d,32f,32p,8e,32c,32p,8d,32a,32p,8g,32c,32p,8d,32f,32p,8e,32c,32p,8g,32b,32p,8c6,32a#,32p,8g#,32g,32p,8f,32d#,32p,8d,32a#4,32p,8a#,32c6,32p,8a#,32g,32p,16a,16f,16d,16g,16e,16d";
            const char * xmen = "xmen:d=4,o=6,b=140:16f#5,16g5,16b5,16d,c#,8b5,8f#5,p,16f#5,16g5,16b5,16d,c#,8b5,8g5,p,16f#5,16g5,16b5,16d,c#,8b5,8d,2p,8c#,8b5,2p,16b5,16e,16f#,16g,f#,8e,8b5,p,16b5,16e,16f#,16g,f#,8e,8c,p,16f#5,16g5,16b5,16d,c#,8b5,8d,2p,8c#,8b5,2p";
            const char * beethoven = "Beethoven - Fur Elise : d=4,o=5,b=140:8e6,8d#6,8e6,8d#6,8e6,8b,8d6,8c6,a,8p,8c,8e,8a,b,8p,8e,8g#,8b,c6,p,8e,8e6,8d#6,8e6,8d#6,8e6,8b,8d6,8c6,a,8p,8c,8e,8a,b,8p,8e,8c6,8b,2a";
            const char * puffs = "Powerpuf:d=4,o=5,b=200:8c,p,8c,8p,8d#,8g,8a#,a.,8g,2p,8c6,p,8c6,8p,8d#6,8g6,8a#6,a#.6,8c7,p,8p,8c6,8c,p,8c,8p,8d#,8g,8a#,2c.6,p,1c,d#.,c.,g,1f#,p,8g,8c6";
            const char * westmin15 = "westmin-15:d=4,o=6,b=120:g#,f#,e,2b4!";
            const char * westmin30 = "westmin-30:d=4,o=6,b=120:e,g#,f#,2b4!,e,f#,g#,2e!";
            const char * westmin45 = "westmin-45:d=4,o=6,b=120:g#,e,f#,2b4!,b3,f#,g#,2e!,g#,f#,e,2b4!";
            const char * westminhour = "westmin-hour:d=4,o=6,b=120:e,g#,f#,2b4!,e,f#,g#,2e!,g#,e,f#,2b4!,b4,f#,g#,2e!";
            const char * westminbong = "westmin-bong:d=4,o=6,b=220:1e3!";
            const char * wonderfulworld = "WonderfulWorld:d=4,o=5,b=110:32p,8c.,8e,f.,8a,2c.6,d6,16d6,16d6,2c.6,8a#.,8a#,16a#,8a.,8a.,8a,8g.,8g,8g,16g,f.,8f,16f,8f.,8f.,8f,2f.,8f.,16f,8e.,8f.,8g,1a,8c.,8e,f.,8a,2c.6,d6,16d6,16d6,c.6,8f,a#.,8a#,16a#,a.,8a,g.,16g,16g,f.,16f,16f,8f.,8f.,8f,2f.,16f,16f,8e,8f.,8g.,1f";
         } songs;


    public:
        const char * getStartUpSong();
        const char * getSongByName(const char *);
        const char * getSongForAlarm(int, int);
        const char * getSpecialHourChime(int, int);
        DynamicJsonDocument getListOfSongs();
};