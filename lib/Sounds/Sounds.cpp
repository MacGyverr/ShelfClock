#include <Sounds.h>

const char *  Sounds::getStartUpSong() {
    return songs.smb_under;
}

const char * Sounds::getSongForAlarm(int day, int month) {
    if (month == 9 && day == 7) return songs.birthday;
    if (month == 8 && day == 13) return songs.birthday;
    if (month == 3 && day == 19) return songs.birthday;
    if (month == 5 && day == 4) return songs.starwars;
    if (month == 12 && day == 25) return songs.xmas;
    if (month == 5 && day == 5) return songs.cinco;
    if (month == 10 && day == 31) return songs.halloween;

    int rand = random(20);
    switch (rand) {
        case 1: return songs.puffs;
        case 2: return songs.adams;
        case 3: return songs.burgertime;
        case 4: return songs.rickroll2;
        case 5: return songs.melody;
        case 6: return songs.rickroll;
        case 7: return songs.mario;
        case 8: return songs.mspacman;
        case 9: return songs.xmen;
        case 10: return songs.galaga;
        default: return songs.finalcount;
    }
}

const char * Sounds::getSongByName(const char* song) {
    if (strncmp("rickroll2", song, 20) == 0) return songs.rickroll2;
    else if (strncmp("auldlang", song, 20) == 0) return songs.auldlang;
    else if (strncmp("startrek", song, 20) == 0) return songs.startrek;
    else if (strncmp("starwars", song, 20) == 0) return songs.starwars;
    else if (strncmp("birthday", song, 20) == 0) return songs.birthday;
    else if (strncmp("rickroll", song, 20) == 0) return songs.rickroll;
    else if (strncmp("cinco", song, 20) == 0) return songs.cinco;
    else if (strncmp("xmas", song, 20) == 0) return songs.xmas;
    else if (strncmp("macgyver", song, 20) == 0) return songs.macgyver;
    else if (strncmp("takeonme", song, 20) == 0) return songs.takeonme;
    else if (strncmp("melody", song, 20) == 0) return songs.melody;
    else if (strncmp("halloween", song, 20) == 0) return songs.halloween;
    else if (strncmp("mandy", song, 20) == 0) return songs.mandy;
    else if (strncmp("burgertime", song, 20) == 0) return songs.burgertime;
    else if (strncmp("mario", song, 20) == 0) return songs.mario;
    else if (strncmp("smb_under", song, 20) == 0) return songs.smb_under;
    else if (strncmp("finalcount", song, 20) == 0) return songs.finalcount;
    else if (strncmp("adams", song, 20) == 0) return songs.adams;
    else if (strncmp("mspacman", song, 20) == 0) return songs.mspacman;
    else if (strncmp("galaga", song, 20) == 0) return songs.galaga;
    else if (strncmp("xmen", song, 20) == 0) return songs.xmen;
    else if (strncmp("beethoven", song, 20) == 0) return songs.beethoven;
    else if (strncmp("puffs", song, 20) == 0) return songs.puffs;
    else if (strncmp("westmin15", song, 20) == 0) return songs.westmin15;
    else if (strncmp("westmin30", song, 20) == 0) return songs.westmin30;
    else if (strncmp("westmin45", song, 20) == 0) return songs.westmin45;
    else if (strncmp("westminhour", song, 20) == 0) return songs.westminhour;
    else if (strncmp("westminbong", song, 20) == 0) return songs.westminbong;
    return nullptr;
}