#include "mfoc.h"
#include <arduino.h>
#include <SPI.h>
#include "card.h"




// Array with default Mifare Classic keys
uchar defaultKeys[][6] = {
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // User defined key slot
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // Default key (first key used by program if no user defined key)
	{0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // NFCForum MAD key
	{0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // NFCForum content key
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Blank key
	{0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5},
	{0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd},
	{0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a},
	{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
	{0x71, 0x4c, 0x5c, 0x88, 0x6e, 0x97},
	{0x58, 0x7e, 0xe5, 0xf9, 0x35, 0x0f},
	{0xa0, 0x47, 0x8c, 0xc3, 0x90, 0x91},
	{0x53, 0x3c, 0xb6, 0xc7, 0x23, 0xf6},
	{0x8f, 0xd0, 0xa4, 0xf2, 0x56, 0xe9}
};


int trailer_block(uint block)
{
	// Test if we are in the small or big sectors
	return (block < 128) ? ((block + 1) % 4 == 0) : ((block + 1) % 16 == 0); 
}


bool checkKey(uint block, uchar *key, uchar type)
{
    uchar status;
    uchar str[MAX_LEN];
    MFRC522_Init();
    status = MFRC522_Request(PICC_REQIDL, str); 
 	if (status != MI_OK)
    {
        return false;
    }

    status = MFRC522_Anticoll(str);
    if (status != MI_OK)
    {
    	return false;
    }
    MFRC522_SelectTag(str);
    status = MFRC522_Auth(type, block, key, str);
    if (status == MI_OK)
    {
        //Serial.print("Successed!");
        return true;
    }else
    {
        //Serial.print("Failed!");
    }
    return false;
}

void checkDefaultKey()
{
    tag t;

    uchar status;
    uchar str[MAX_LEN];
    MFRC522_Init(); 

    // Search card, return card types
    status = MFRC522_Request(PICC_REQIDL, str); 
    if (status != MI_OK)
    {
        return;
    }
    
    if (str[0] == 0x02 && str[1] == 0x00)
    {
        t.b4K = true;
    }
    else
    {
        t.b4K = false;
    }

    // Show card type
    //ShowCardType(str);
    
    //Prevent conflict, return the 4 bytes Serial number of the card
    status = MFRC522_Anticoll(str);
    
    // str[0..3]: serial number of the card
    // str[4]: XOR checksum of the SN.
    if (status == MI_OK)
    {
        //Serial.print("The card's number is: ");
        memcpy(t.uid, str, 5);

        t.num_blocks = (t.b4K) ? 0xff : 0x3f;
        t.num_sectors = t.b4K ? NR_TRAILERS_4k : NR_TRAILERS_1k;
        MFRC522_Halt(); //command the card into sleep mode 


        for (int block = 0; block <= t.num_blocks; ++block) 
        {
            if (trailer_block(block)) 
            {

                Serial.print("Crack block: ");
                Serial.print(block);
                Serial.println("");

                for (int i = 0; i < 14; ++i)
                {
                    Serial.print("Try Key: ");
                    ShowKey(defaultKeys[i]);
                    //Serial.println("");
                    if (checkKey(block, defaultKeys[i], PICC_AUTHENT1A))
                    {
                        Serial.println("Successed!");
                        break;
                    //    ShowKey(defaultKeys[i]);
                    }else
                    {
                        Serial.println("Failed!");
                    }
                    //Serial.println("");
                }
            }

        }

        Serial.println("Done.");

    }

/*
    blockAddr = 0;
    status = MFRC522_Auth(PICC_AUTHENT1A, blockAddr, defaultKeys[2], t.uid);
    if (status == MI_OK)
    {
      Serial.print("OK!");
    }
    else
    {
      Serial.print("Failed!");
    }
*/
    MFRC522_Halt(); //command the card into sleep mode 

    delay(20);
}




uchar test(uchar *p, uint size, uchar *pback)
{
    uchar status; 
    uint backBits; //the data bits that received
    //uchar *backString;

    //Write_MFRC522(BitFramingReg, 0x07); //TxLastBists = BitFramingReg[2..0] ???
    //Write_MFRC522(BitFramingReg, 0x00);
    
    status = MFRC522_ToCard(PCD_TRANSCEIVE, p, size, pback, &backBits);

    Serial.println(backBits);
    //Serial.println(backString[0], HEX);

    if ((status != MI_OK) || (backBits != 0x10))
    { 
        status = MI_ERR;
    }
   
    return status;
}