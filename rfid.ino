#include <arduino.h>
#include <Regexp.h>
#include <SPI.h>
#include "card.h"
#include "mfoc.h"


#define CMDLEN 50

void setup() 
{ 
    Serial.begin(57600); 
    
    SPI.begin();
    
    pinMode(chipSelectPin,OUTPUT); // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin 
    digitalWrite(chipSelectPin, LOW); // Activate the RFID reader
    pinMode(NRSTPD,OUTPUT); // Set digital pin 5 , Not Reset and Power-down
    
    //MFRC522_Init(); 
}


void loop()
{
    MatchState ms;
    char cmd[CMDLEN] = {0};
    uint index = 0;


    if (ReadCMD(cmd))
    {
        ms.Target(cmd);
        char result = ms.Match ("(%a+)(.*)", index);
        if (result == REGEXP_MATCHED)
        {
            char buf[200];
            //Serial.println(ms.level);
            //for (int i = 0; i < ms.level; ++i)
            //{
            //    Serial.println(ms.GetCapture(buf, i));
            //}

            char *p = ms.GetCapture(buf, 0);

            //Serial.println(p);

            Serial.print(">>>");
            Serial.print(cmd);
            Serial.println("");

            if (strcmp(p, "test") == 0)
            {
                uchar str[MAX_LEN];
                DebugOn();
                MFRC522_Init();

                readdata();
                readdata();
                readdata();
                readdata();
                readdata();
                //uchar buf[] = {0x26};
                /*
                uchar buf[] = {0x04, 0x00};
                Write_MFRC522(BitFramingReg, 0x07); 
                test(buf, 2, str);
                /*
                uchar buf1[] = {0x93, 0x20};
                Write_MFRC522(BitFramingReg, 0x00); 
                test(buf1, 2, str);
                */
                //MFRC522_Halt();
            }
            else if (strcmp(p, "help") == 0 || strcmp(cmd, "?") == 0)
            {
                usage();
            }
            else if (strcmp(p, "show") == 0)
            {
                //Serial.println(ms.GetCapture(buf, 1));
                if (strcmp(ms.GetCapture(buf, 1), " tag") == 0)
                    showTag();
            }
            else if (strcmp(p, "debug") == 0)
            {
                if (strcmp(ms.GetCapture(buf, 1), " off") == 0)
                    DebugOff();
                else
                    DebugOn();
            }
            else if (strcmp(p, "auth") == 0)
            {
                result = ms.Match("auth (%d+) 0x(.+) ([abAB])", index);
                if (result == REGEXP_MATCHED)
                {
                    uchar type;
                    char *ptype = ms.GetCapture(buf, 2);

                    if (strcmp(ptype, "A") == 0 || strcmp(ptype, "a") == 0)
                    {
                        type = PICC_AUTHENT1A;
                    }
                    else
                    {
                        type = PICC_AUTHENT1B;
                    }

                    uint block = atoi(ms.GetCapture(buf, 0));

                    uchar key[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
                    char * pkey = ms.GetCapture(buf, 1);
                    char kk[12] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};

                    for (int i = 0; i < 12; ++i)
                    {
                        if (pkey[i] == NULL)
                        {
                            break;
                        }
                        kk[i] = pkey[i];
                    }

                    for (int i = 0; i < 6; ++i)
                    {
                        char p[2] = {kk[i * 2], kk[i * 2 + 1]};
                        key[i] = strtoul(p, NULL, 16);
                        //Serial.println(s.substring(i * 2, i * 2 + 2));
                        //Serial.print(key[i], HEX);
                        //Serial.print(" ");
                    }
                    //Serial.println("KEY");
 
                    if (checkKey(block, key, type))
                        Serial.println("Successed!");
                    else
                        Serial.println("Failed!");

                }
                else
                {
                    Serial.println("auth <block addr> <key: 0xffffffffffffff> <type: A/B>: verifiy key.");
                }
                //checkKey(uchar *uid, uint block, uchar *key, uchar type);
            }
            else if (strcmp(cmd, "check default key") == 0)
            {
                checkDefaultKey();
            }
            else
            {
                Serial.println("Unkown command!");
            }

            Serial.print("\n");
        }


    }
}


//char *command(char* This) 
bool ReadCMD(char * cmd)
{
    int index = 0;
    bool b = false;

    if (Serial.available())
    {
        b = true;
        /*
        for (int i = 0; i < CMDLEN; ++i)
        {
            cmd[i] = '\0';
        }
        */
    }

    while(b && index < CMDLEN)
    {
            cmd[index] = Serial.read();
            index ++;


            for (int i = 0; i < 20; ++i)
            {
                if (Serial.available())
                {
                    break;
                }

                delay(1);

                if (i >= 19)
                {
                    b = false;
                    break;
                }
            }

    }


    if (index > 0)
    {
        cmd[index] = '\0';
        return true;
    }

    return false;
}

void showTag()
{
    uchar status;
    uchar str[MAX_LEN];
    MFRC522_Init(); 
    status = MFRC522_Request(PICC_REQIDL, str); 
    if (status != MI_OK)
    {
        Serial.println("Show Tag Informations Failed!");
        return;
    }

    // Show card type
    ShowCardType(str);
    
    //Prevent conflict, return the 4 bytes Serial number of the card
    status = MFRC522_Anticoll(str);
    
    // str[0..3]: serial number of the card
    // str[4]: XOR checksum of the SN.
    if (status == MI_OK)
    {
        ShowCardID(str);
    }

    MFRC522_Halt();
}


void usage()
{
    Serial.println("NFC Tools (version 1.0): by R5");
    Serial.println("help: help");
    Serial.println("show tag: show the tag informations.");
    Serial.println("check default key: Check password by default.");
    Serial.println("check password by default: Check password by default.");
    Serial.println("auth <block addr> <key: 0xffffffffffffff> <type: A/B>: verifiy key.");
    Serial.println("read <block addr> <keyA> [keyB]");


}