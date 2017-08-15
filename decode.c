#include "decode.h"
#include "splay.h"

struct zerg * create_zerg(unsigned int dest_id);
void destroy_zerg(struct zerg * old_zerg);


uint16_t get_root_srcID(void * data)
{
	return ((struct zerg*)data)->srcID;
} 


void display(const void * data)
{
	printf("display node\n");
	printf("zerg_id: %u \n", ((struct zerg*)data)->srcID );
	printf("zerg_id: %06f \n",  ((struct zerg*)data)->position.latitude.value );
	printf("zerg_id: %06f \n",  ((struct zerg*)data)->position.longitude.value );
	printf("zerg_id: %06f \n",  ((struct zerg*)data)->position.altitude.value );
	printf("\n");
}


struct node * decode (int argc, char **argv, int node_count)
{
	FILE *fp;
	struct zergPacket pcapfile;
	int file, msgType, filesize, sum;
	char filename[100];
	if (argc < 2)
	{
		fprintf(stderr, "%s: usage error < input file >\n", argv[0]);
		exit(1);
	}
	
	struct node * root = NULL;
	//struct node * root = tree->head;
	
	
	
	//Iterate pcap build for a sequence of files from the command line
	for ( file = 1; file < argc; file++) 
	{
		//Fill pcap structure with individual header structures.
		strcpy(filename, argv[file]);
		fp = buildPcapData(&pcapfile, filename, &filesize);
		//Attempt to as many pcaps in a singe file.
		while((ftell(fp) < filesize) && ((filesize - ftell(fp)) > 60))
		{
			
			fp = buildPacketData(&pcapfile, fp);
			//printf("\nVersion: %x \n", htonl(pcapfile.pcapZerg.ver_type_totalLen) >> 28);
			//printf("Sequence: %u \n", htonl(pcapfile.pcapZerg.seqID));
			//printf("From: %u\n", htons(pcapfile.pcapZerg.destID));
			//printf("To: %u\n", htons(pcapfile.pcapZerg.sourceID));
			msgType = ((htonl(pcapfile.pcapZerg.ver_type_totalLen) >> 24) & 0x0f);
			
			struct zerg * zerg_info = create_zerg( htons(pcapfile.pcapZerg.destID) );
			
			switch (msgType)
			{
				case 0:
					fp = printMsgPayload(&pcapfile, fp);
					break;
				case 1:
					fp = fillStatusPayload(&pcapfile, fp, zerg_info);
					break;
				case 2:
					fp = printCmdPayload(fp);
					break;
				case 3:
					printf("getting gps data\n");
					fp = fillGpsPayload(fp, zerg_info);
					break;
				default:
					printf("Unknown payload type\n");
					break;
			}
			
			/* Build a zerg_info structure for the pcap that was read */
			/*
			printf("zerg_info = null: %c \n\n", zerg_info != NULL ? 'F':'T');	
			printf("zerg : %u \n", zerg_info->srcID );	
			printf("zerg long: %f \n", zerg_info->position.longitude.value);
			printf("zerg lat: %f \n", zerg_info->position.latitude.value);	
			printf("zerg alt: %f \n\n", zerg_info->position.altitude.value);	
			*/
			root = insert(root, zerg_info->srcID, sizeof(struct zerg), get_root_srcID);
			node_count++;
			//printf("key = null ? %c \n", (((struct zerg*)root->key)->srcID) == 0 ? 'T': 'F');
			if(   ! (((struct zerg*)root->key)->srcID)  )
			//if root = null -> new node then key == null
			{
				printf("here!\n");
				((struct zerg*)root->key)->srcID = zerg_info->srcID;
				((struct zerg*)root->key)->position.latitude.value = zerg_info->position.latitude.value;
				((struct zerg*)root->key)->position.longitude.value = zerg_info->position.longitude.value;
				((struct zerg*)root->key)->position.altitude.value = zerg_info->position.altitude.value;
				/* ADD zerg_struct data to key */
			}
			else // Key != NULL 
			{
				printf("or here!\n");
				/* modifiy the data in the root->key data fields */
			}
			
			
			//HANDLE PADDING OR BLANK FCS at end of the pcap.
			fread(filename, 1, 4, fp);
			for (unsigned int x = 0; x < strlen(filename); x++)
			{
				sum += filename[x];
				if (sum != 0)
				{
					fseek(fp, -4, SEEK_CUR);
					break;
				}
			}
		}
		fclose(fp);
	}
	
		//PRINT OUT all the zerg_info structs
	//printf("\n\nprinting nodes inside decode\n\n");
	//preOrder(root, display);
	
	
	//For test return 1 for success
	return root;
}

struct zerg * create_zerg(unsigned int src_id)
{
	struct zerg * new_zerg = malloc(sizeof(struct zerg));
	new_zerg->srcID = src_id;
	
	return new_zerg;	
	
}

void destroy_zerg(struct zerg * old_zerg)
{
	if( old_zerg != NULL)
	{
		free(old_zerg->zerg_name);
		free(old_zerg);
	}
}



