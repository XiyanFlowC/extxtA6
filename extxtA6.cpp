#include <string.h> 
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#define ADDR2OFST(addr) ((long)((addr) - 0xFF000))
#define OFST2ADDR(ofst) ((long)((ofst) + 0xFF000))

#define PTR2OFST(buf, ptr) ((long)((long long)ptr - (long long)buf))

#define DREF(buf, addr) ((void*)((buf) + ADDR2OFST((addr)))) //translate elf's pointer to pointer on buffer
#define EREF(buf, ptr) ((long)(OFST2ADDR((long long)ptr - (long long)buf)))

typedef unsigned char byte_t;

struct slctgrp //select group
{
	long    ppc_selections;//ended with NULL(?
};

struct dialogue//9caa60
{
	long    flags;//may be display control(loc
	long    id;
	long    pc_text;
};

struct event_header
{
	long      grpno;//related with group
	long      ps_dialogues;
	long      pl_scriptA;
	long      pl_scriptB;
	long      ps_slctgrps;
	long      pc_evname;
	long      pc_evmemo;
	long      unk2;//maybe padding
};

struct effect_term
{
	long pc_name;
	long unk;
};

struct mana_item_term
{
	long   pc_name;
	byte_t item_info[124];
	long   pc_desc;
	byte_t padding[12];
};

struct item_term
{
	long   pc_name;
	long   pc_aka;
	byte_t item_info[152];
	long   pc_desc;
	byte_t padding[12];
};

struct skill_term
{
	long   pc_name;
	long   pc_show;
	byte_t skill_info[108];
	long   pc_desc;
	byte_t padding[8];
};

struct quest_hint_term
{
	long   unk;
	long   pc_text;
}; 

//event_header* event_table = ADDR2OFST(0xB1DFE8);//addr
//311 terms
//event_header* launcher = event_table[0x253];

byte_t* buffer;

void extract_events();
void extract_effects();
void extract_mana_item();
void extract_item();
void extract_skill();
void extract_quest_hint();
void extract_enemy_skill();
void extract_meld();

int main(int argc, char ** argv)
{
	if(argc != 2)
	{
		printf("Usage: %s [ELF path]\n", argv[0]);
		return 0;
	}
	
    FILE* elf = fopen(argv[1], "rb" );//start: load file to memory 
    if(elf == NULL)
    {
    	int err = errno;
        fprintf(stderr, "Can't open file. Errno: %d\n", err);
        return err;
    }

    fseek (elf, 0, SEEK_END);//get file size
    long size = ftell(elf);
    rewind (elf);

    buffer = (byte_t*)malloc(sizeof(char)*size);
    if (buffer == NULL)
    {
    	int err = errno;
        fprintf(stderr, "Can't alloc memory. Error: %d", err);
        return err;
    }

    int bytes = fread(buffer, 1, size, elf);
    if (bytes != size)
    {
        fprintf(stderr, "Read error.");
        return -1;
    }
    fclose(elf);//end: load file to memory

	printf("Ready to extract.\n");

	printf("Extract event data...\n");
	extract_events();
	
	printf("Extract effect info...\n");
	extract_effects();
	
	printf("Extract mana item info...\n");

	printf("Extract item info...\n");
	extract_item();
	
	printf("Extract skill info...\n");
	extract_skill();
	
	printf("Extract quest hint...\n");
	extract_quest_hint();
	
	printf("Extract enemy skill...\n");
	extract_enemy_skill();
	
	printf("Extract meld...\n");
	extract_meld();
	
	printf("Extract etc...\n");
	FILE* ukn = fopen("ukn.txt", "w");
	
	long* etc = (long*)(buffer + 0xB161A0);
	for(int i = 0; i < 100; ++i)
	{
		//printf("%X\n", etc[i]);
		fprintf(ukn, "0x%X,%s\n\n",
			PTR2OFST(buffer, etc + i),
			(char*)DREF(buffer, etc[i]));
	}
	
	fprintf(ukn, "==========\n");
	
	etc = (long*)(buffer + 0xB16070);
	for(int i = 0; i < 17; ++i)
	{
		//printf("%X\n", etc[i]);
		fprintf(ukn, "0x%X,%s\n\n",
			PTR2OFST(buffer, etc + i),
			(char*)DREF(buffer, etc[i]));
	}
	
	fprintf(ukn, "==========\n");
	
	etc = (long*)(buffer + 0xB16050);
	for(int i = 0; i < 6; ++i)
	{
		//printf("%X\n", etc[i]);
		fprintf(ukn, "0x%X,%s\n\n",
			PTR2OFST(buffer, etc + i),
			(char*)DREF(buffer, etc[i]));
	}
	
	fclose(ukn);
	
	free(buffer);
	return 0;
}

void extract_enemy_skill()
{
	FILE* skill = fopen("enemyskill.txt", "w");
	
	skill_term* sth = (skill_term*)(buffer + 0x847DB0);
	for(int i = 0; i < 192; ++i)
	{
		printf("%d\r", i);
		fprintf(skill, "0x%X,%s\n0x%X,%s\n\n0x%X,%s\n----------\n",
			PTR2OFST(buffer, &sth[i].pc_name),   //offset of pointer point to string
			(char*)DREF(buffer, sth[i].pc_name), //inner name
			PTR2OFST(buffer, &sth[i].pc_show),
			(char*)DREF(buffer, sth[i].pc_show), //display name
			PTR2OFST(buffer, &sth[i].pc_desc),
			(char*)DREF(buffer, sth[i].pc_desc)); //description
	}
	
	fclose(skill);
}

void extract_quest_hint()
{
	FILE* hint = fopen("questhint.txt", "w");
	
	long* hint_ptr = (long*)DREF(buffer, 0xB2E160);
	
	for(int i = 0; i < 28; ++i)
	{
		quest_hint_term* qht = (quest_hint_term*)DREF(buffer, hint_ptr[i]);
		
		while(qht->pc_text)
		{
			fprintf(hint, "0x%X,0x%X,%s\n\n",
				PTR2OFST(buffer, &qht->pc_text),
				qht->unk,
				(char*)DREF(buffer, qht->pc_text));
			qht++;
		}
		
		fprintf(hint, "==========\n");
	}
	fclose(hint);
}

void extract_skill()
{
	FILE* skill = fopen("skill.txt", "w");
	
	skill_term* sth = (skill_term*)(buffer + 0x84DDB0);
	for(int i = 0; i < 189; ++i)
	{
		fprintf(skill, "0x%X,%s\n0x%X,%s\n\n0x%X,%s\n----------\n",
			PTR2OFST(buffer, &sth[i].pc_name),   //offset of pointer point to string
			(char*)DREF(buffer, sth[i].pc_name), //inner name
			PTR2OFST(buffer, &sth[i].pc_show),
			(char*)DREF(buffer, sth[i].pc_show), //display name
			PTR2OFST(buffer, &sth[i].pc_desc),
			(char*)DREF(buffer, sth[i].pc_desc)); //description
	}
	
	fclose(skill);
}

void extract_item()
{
	FILE* skill = fopen("item.txt", "w");
	
	item_term* sth = (item_term*)(buffer + 0x867B00);
	for(int i = 0; i < 0x1BC; ++i)
	{
		fprintf(skill, "0x%X,%s\n0x%X,%s\n\n0x%X,%s\n----------\n",
			PTR2OFST(buffer, &sth[i].pc_name),  //offset of pointer point to string
			(char*)DREF(buffer, sth[i].pc_name),//inner name
			PTR2OFST(buffer, &sth[i].pc_aka),
			(char*)DREF(buffer, sth[i].pc_aka), //aka
			PTR2OFST(buffer, &sth[i].pc_desc),
			(char*)DREF(buffer, sth[i].pc_desc)); //description
	}
	
	fclose(skill);
}

void extract_mana_item()
{
	FILE* skill = fopen("manaitem.txt", "w");
	
	mana_item_term* sth = (mana_item_term*)(buffer + 0x866A00);
	for(int i = 0; i < 0x1e; ++i)
	{
		fprintf(skill, "0x%X,%s\n\n0x%X,%s\n----------\n",
			PTR2OFST(buffer, &sth[i].pc_name),
			(char*)DREF(buffer, sth[i].pc_name),
			PTR2OFST(buffer, &sth[i].pc_desc),
			(char*)DREF(buffer, sth[i].pc_desc));
	}
	
	fclose(skill);
}

void extract_effects()
{
	FILE* effect = fopen("effect.txt", "w");
	
	effect_term* efi = (effect_term*)(buffer + 0x866640);
	while(efi->pc_name)
	{
		fprintf(effect, "0x%X,0x%X,%s\n\n",
			PTR2OFST(buffer, efi), //offset of pointer point to string
			efi->unk,           //unknown info, maybe useful
			(char*)DREF(buffer, efi->pc_name)); //text
		++efi;
	}
	
	fclose(effect);
}

void extract_meld()
{
	FILE* meld = fopen("meld.txt", "w");
	
	long* ptr = (long*)(buffer + ADDR2OFST(0x985610));
	for(int i = 0; i < 14; ++i)
	{
		fprintf(meld, "=====[%d]=====\n\n", i);
		
		dialogue* dial = (dialogue*)DREF(buffer, *ptr++);
		while(dial->flags)
		{
			if(dial->pc_text > OFST2ADDR(0xCE4620) ||
				dial->pc_text < OFST2ADDR(0xBDF4F0)) break;//can't find ending flag, so just
				//terminate if the pointer is invalid or unacceptable
			
			fprintf(meld, "0x%X,0x%X,%s\n\n",
				(((long long)dial) - ((long long)buffer)), //the offset of pointer point to string
				dial->flags,     //display control, could be useful if need to know who is talking
				(char*)DREF(buffer, dial->pc_text)); //the dialogue text
			
			++dial;
		}
	}
	
	fclose(meld);
}

void extract_events()
{
	long* hds = (long*)(buffer + ADDR2OFST(0xB1DFE8));//set event header
	FILE* memo = fopen("event.txt", "w");
	
	for(int i = 0; i < 0x311; ++i)
	{
		event_header* evh = (event_header*)DREF(buffer, hds[i]);
		/*printf("%d: %s - %s\n", i,
			(char*)DREF(buffer, evh->pc_evname),
			(char*)DREF(buffer, evh->pc_evmemo));*/
		fprintf(memo, "%d: %s - %s\n", i,
			(char*)DREF(buffer, evh->pc_evname),
			(char*)DREF(buffer, evh->pc_evmemo));
		
		static char fnbuf[128];
		strcpy(fnbuf, "Event/");
		FILE* event = fopen(strcat(strcat(fnbuf, ((char*)DREF(buffer, evh->pc_evname))), ".txt"), "w");
		
		fprintf(event, "%s\n===============\n", DREF(buffer, evh->pc_evmemo));
		//printf("Extract dialogues...\n");
		dialogue* dial = (dialogue*)DREF(buffer, evh->ps_dialogues);
		while(dial->flags)
		{
			if(dial->pc_text > OFST2ADDR(0xCE4620) ||
				dial->pc_text < OFST2ADDR(0xBDF4F0)) break;//can't find ending flag, so just
				//terminate if the pointer is invalid or unacceptable
			
			fprintf(event, "0x%X,0x%X,%s\n\n",
				(((long long)dial) - ((long long)buffer)), //the offset of pointer point to string
				dial->flags,     //display control, could be useful if need to know who is talking
				(char*)DREF(buffer, dial->pc_text)); //the dialogue text
			
			//if(dial->flags & 0x1000) break;//no, this flag declare if display portrait or not
			++dial;
			
			//fprintf(stderr, "%X\n", dial->pc_text);
		}
		
		fprintf(event, "Selections\n===============\n");
		//printf("Extract selections...\n");
		if(evh->ps_slctgrps == 0x0) //if header declare no selection
		{
			fclose(event);
			continue;
		}
		slctgrp* sgs = (slctgrp*)DREF(buffer, evh->ps_slctgrps);
		int C = 0;
		while(sgs->ppc_selections)
		{
			fprintf(event, "----[%d]----\n", C++);
			
			if(sgs->ppc_selections > OFST2ADDR(0xCE4620) ||
				sgs->ppc_selections < OFST2ADDR(0x100000)) break;//if the pointer point to somewhere unacceptable
			
			long* pcslct = (long*)DREF(buffer, sgs->ppc_selections);
			while(*pcslct)
			{
				if(*pcslct > OFST2ADDR(0xCE4620) ||
					*pcslct < OFST2ADDR(0xBDF4F0)) break;//if the pointer point to somewhere unacceptable
				
				fprintf(event, "0x%X,%s\n\n",
					(((long long)pcslct) - ((long long)buffer)), //offset of pointer point to string
					(char*)DREF(buffer, *pcslct++));             //selection text
			}
			sgs++;
			//if(sgs->ppc_selections < 0 || sgs->ppc_selections > size) break;
		}
		
		fclose(event);
	}
	
	fclose(memo);
}
