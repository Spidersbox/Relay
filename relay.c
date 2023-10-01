/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <hidapi/hidapi.h>
#include "relay.h"

#define VERSION "1.0"
#define DATE "Sept. 13, 2015"


int main( int argc, char *argv[])
{
  unsigned char buf[9];// 1 extra byte for the report ID
  struct hid_device_info *devs, *cur_dev;
  hid_device *handle;
  unsigned short vendor_id  = 0x16c0;
  unsigned short product_id = 0x05df;
  char *vendor, *product;

  int  total_cards=0;
  int  total_relays=0;

  devs = hid_enumerate(vendor_id, product_id);

  struct hid_device_info *temp_dev;
  cur_dev = devs;
  temp_dev=devs;

//  count the cards
  int cnum=0;
  while(temp_dev)
  {
    cnum++;
    temp_dev=temp_dev->next;
  }
  free(temp_dev);
  temp_dev=NULL;

  total_cards=cnum;

  struct cards card[total_cards];


// gather all the information and stuff in to strut card array
  cnum=0;

  while (cur_dev)
  {
    card[cnum].vendor=cur_dev->vendor_id;
    card[cnum].product=cur_dev->product_id;
    strcpy(card[cnum].path,cur_dev->path);
    card[cnum].relays=atoi((const char *)&cur_dev->product_string[8]);
    int i;
    total_relays=0;
    for(i=0;i<total_cards;i++)
      total_relays=total_relays+card[i].relays;

    handle = hid_open_path(cur_dev->path);
    if (!handle)
    {
      printf("unable to open device\n");
      return 1;
    }
    buf[0] = 0x01;
    int ret = hid_get_feature_report(handle,buf,sizeof(buf));
    if (ret == -1)
    {
      printf("unable to get hid_get_feature_report");
      exit(1);
    }
    for(i=0;i<10;i++)
      card[cnum].sn[i]=buf[i];

    hid_close(handle);
    cnum++;
    cur_dev = cur_dev->next;
  }// end while


//*************************************************
//*****************  sort cards *******************
/*
  struct cards temp[1];
  int i;
  for(i=0;i<total_cards;i++)
  {
    if(card[i].sn[0] > card[i+1].sn[0])
    {
      temp[0]=card[i];
      card[i]=card[i+1];
      card[i+1]=temp[0];
    }
  }
*/
//**************************************************
//***************  parse commands  *****************

  if(argc==1)
    ListCommands();

  if(argc>1)
  {
    if(!strcasecmp(argv[1],"-i"))
      ListDevices(card,total_cards);

// try to turn the first arg into an init.
    int search=atoi(argv[1]);

    if(search) // if it is an int larger than 0
    {
      if(search>total_relays)
      {
        printf("out of bounds: %d\n",search);
        printf("accessible relays 1 - %d\n",total_relays);
      }
      else
      { //  relay number is ok
        if(argc==2) // get state of relay #
        {
          int state=GetRelay(card,search,total_cards);
          printf("Relay %d is %s\n",search,(state)?"ON":"OFF");
        }
        if(argc==3) 
        {
          if(!strcasecmp(argv[2],"on"))
          {
            SetRelay(card,search,ON,total_cards);
          }
          else if(!strcasecmp(argv[2],"off"))
          {
            SetRelay(card,search,OFF,total_cards);
          }
          else if(!strcasecmp(argv[2],"pulse"))
          {
            //printf("relay on ...(wait)\n");
            PulseRelay(card,search,total_cards);
            //printf("relay off.\n");
          }
          else
          {
            printf("SN error. I don't understand %s.\n",argv[2]);
          }
        }
      }
    }
    else // must be a word like all or pulse 
    {
      if(!strcasecmp(argv[1],"all"))
      {
        if(argc==2)
        { // nothing follows all so list state of all relays
          printf("\n");
          int i;
          for(i=0;i<total_relays;i++)
          {
            int state=GetRelay(card,i+1,total_cards);
            printf("Relay %d is %s\n",i+1,(state)?"ON":"OFF");
          }  
        }
        else
        { //more to come
          if(argc==3)
          {
            if(!strcasecmp(argv[2],"on"))
            {
              SetAllRelays(card,ON,total_cards);
            }
            else if(!strcasecmp(argv[2],"off"))
            {
              SetAllRelays(card,OFF,total_cards);
            }
            else if(!strcasecmp(argv[2],"pulse"))
            {
              //printf("relay on ...(wait)\n");
              PulseAllRelays(card,total_cards);
              //printf("relay off.\n");
            }
            else
            {
              printf("SN error. I don't understand %s.\n",argv[2]);
            }
          }
        }
      }// endif argv[1]==all
//printf("command is: %s.\n",argv[1]);
    }
  }

  hid_free_enumeration(devs);
  /* Free static HIDAPI objects. */
  hid_exit();


  if(cur_dev)
    free(cur_dev);
  cur_dev=NULL;
  exit(0);
}

//----------------------------------------------------------------------------
int GetRelay(struct cards card[10],int search,int total_cards)
{
  int t;
  int relay=0;
  int rtn=0;
  for(t=0;t<total_cards;t++)
  {
    int i;
    for(i=0;i<card[t].relays;i++)
    {
      relay++;
      if(relay==search)
      {
        if (card[t].sn[7] & 1 << i)
        {
          rtn=1;
        }
      }
    }
  }
  return(rtn); //  true
}
//----------------------------------------------------------------------------
int SetRelay(struct cards card[10],int search,unsigned char state,int total_cards)
{
  int t;
  int relay=0;
  int rtn=0;
  for(t=0;t<total_cards;t++)
  {
    int i;
    for(i=0;i<card[t].relays;i++)
    {
      relay++;
      if(relay==search)
      {
        unsigned char buf[9];

        hid_device *handle = hid_open_path(card[t].path);
        if (!handle)
        {
          printf("SetRelay:  unable to open device\n");
          return -1;
        }
        operate_relay(handle,i+1,state);
        hid_close(handle);
      }
    }
  }
  return 0; //  true
}
//----------------------------------------------------------------------------
int SetAllRelays(struct cards card[10],unsigned char state,int total_cards)
{
  int t;
  int relay=0;
  int rtn=0;
  for(t=0;t<total_cards;t++)
  {
    int i;
    for(i=0;i<card[t].relays;i++)
    {
      relay++;
      unsigned char buf[9];

      hid_device *handle = hid_open_path(card[t].path);
      if (!handle)
      {
        printf("SetRelay:  unable to open device\n");
        return -1;
      }
      operate_relay(handle,i+1,state);
      hid_close(handle);
    }
  }
  return 0; //  true
}
//----------------------------------------------------------------------------
void PulseRelay(struct cards card[10],int search,int total_cards)
{
  SetRelay(card,search,ON,total_cards);
  sleep(10);
  SetRelay(card,search,OFF,total_cards);
}
//----------------------------------------------------------------------------
int PulseAllRelays(struct cards card[10],int total_cards)
{
  int t;
  int relay=0;
  int rtn=0;
  for(t=0;t<total_cards;t++)
  {
    int i;
    for(i=0;i<card[t].relays;i++)
    {
      relay++;
      unsigned char buf[9];

      hid_device *handle = hid_open_path(card[t].path);
      if (!handle)
      {
        printf("SetRelay:  unable to open device\n");
        return -1;
      }
      operate_relay(handle,i+1,ON);
      hid_close(handle);
    }
  }
  
  sleep(10);
  for(t=0;t<total_cards;t++)
  {
    int i;
    for(i=0;i<card[t].relays;i++)
    {
      relay++;
      unsigned char buf[9];

      hid_device *handle = hid_open_path(card[t].path);
      if (!handle)
      {
        printf("SetRelay:  unable to open device\n");
        return -1;
      }
      operate_relay(handle,i+1,OFF);
      hid_close(handle);
    }
  }
  return 0; //  true
}

//----------------------------------------------------------------------------
void ListCommands()
{
   printf("relay, version %s  date %s\n",VERSION,DATE);
   printf("Use to control one or more HIDAPI relay cards.\n");
   printf("\n");
   printf("relay [-i] card information\n");
   printf("relay [relay number] check the state of the relay.\n");
   printf("relay [relay number] [on | off] to set the state of the relay.\n");
   printf("relay [relay number] [pulse] to turn on for 10 seconds, then off.\n\n");
   printf("relay [all] returns the state of all relays.\n");
   printf("relay [all] [on | off] to set the state of all the relays.\n");
   printf("relay [all] [pulse] to turn on all relays for 10 seconds, then off.\n\n");
   printf("       The state of any relay can be read or it can be changed to a new state.\n");
   printf("       If only the relay number is provided then the current state is returned,\n");
   printf("       otherwise the relays state is set to the new value provided as second parameter.\n");
  printf("\n");
}
//----------------------------------------------------------------------------
void ListDevices(struct cards card[10],int total_cards) 
{
  int t;
  int relay=0;
  for(t=0;t<total_cards;t++)
  {
    printf("Device Found\n");
    printf("  Type: %04hx %04hx\n",card[t].vendor,card[t].product);
    printf("  Path: %s\n",card[t].path);
    printf("  Number of Relays = %d\n",card[t].relays);
    printf("  Device ID: %s\n",card[t].sn);
    int i;
    for(i=0;i<card[t].relays;i++)
    {
      relay++;
      if (card[t].sn[7] & 1 << i)
      {
        printf("  relay %d is ON\n",relay);
      }
      else
      {
        printf("  relay %d is OFF\n",relay);
      }
    }
  printf("\n");
}
return;
}
//----------------------------------------------------------------------------
int operate_relay(hid_device *handle,unsigned char relay, unsigned char state)
{
  unsigned char buf[9];// 1 extra byte for the report ID
  int res;
//printf("write to relay %02hx to state %02hx \n",relay,state);
  buf[0] = 0x0; //report number
  buf[1] = state;
  buf[2] = relay;
  buf[3] = 0x00;
  buf[4] = 0x00;
  buf[5] = 0x00;
  buf[6] = 0x00;
  buf[7] = 0x00;
  buf[8] = 0x00;
  res = hid_write(handle, buf, sizeof(buf));
  if (res < 0)
  {
    fprintf(stderr,"Unable to write()\n");
    fprintf(stderr,"Error: %ls\n", hid_error(handle));
  }
  return(res);
}
