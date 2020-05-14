#ifndef __Library__

#define __Library__

    /*--------------------------*/

        //  Hodnoty zprav

    /*--------------------------*/
    /* { */

    // -- Zpravy pro prijem

    #define CI_VSTUP_CT 20
    #define CS_VSTUP_CT "Chci cist"
    #define CI_VSTUP_SP 21
    #define CS_VSTUP_SP "Chci psat"
    #define CI_DOBA_CT 22
    #define CS_DOBA_CT "Chci cist od"
    //#define CS_DOBA_CT "Chci cist od %d do %d"
    #define CI_DOBA_SP 23
    #define CS_DOBA_SP "Zapis"
    //#define CS_DOBA_SP "Zapis %s na %d"
    #define CI_KONEC_BOTH 24
    #define CS_KONEC_BOTH "Konec"
    #define CI_VYNUCENY_KONEC_BOTH 25
    #define CS_VYNUCENY_KONEC_BOTH "Vynuceny konec"

    // -- Zpravy pro odeslani

    #define CI_VYSTUP_CT 30
    #define CS_VYSTUP_CT "Muzes cist"
    #define CI_VYSTUP_SP 31
    #define CS_VYSTUP_SP "Muzes psat"
    #define CI_DATA_CT 32
    #define CS_DATA_CT "Ctena data"
    #define CI_HOTOVO_CT 33
    #define CS_HOTOVO_CT "Hotovo"
    #define CI_NASCHLEDANOU_BOTH 34
    #define CS_NASCHLEDANOU_BOTH "Naschledanou Priste"

    // -- Zpravy pro chyby

    #define CI_KLIENT 40
    #define CS_KLIENT "Obecna chyba klienta"
    #define CI_SERVER 41
    #define CS_SERVER "Obecna chyba serveru"
    #define CI_OBSAZENO 42
    #define CS_OBSAZENO "Obsazeno"

    /* } */

    /*--------------------------*/

        //  Možnosti zprav

    /*--------------------------*/
    /* { */

    #define LOG_ERROR               0       // errors
    #define LOG_INFO                1       // information and notifications
    #define LOG_DEBUG               2       // debug messages

    /* } */

    #define NUM_FD 2048 // Pocet fileDescriptoru

#endif