/* NOTES: 
 * pread 	write/read from a file descriptor given an offset
 * dprintf 	output to file descriptor, **SLOW**
 */

/* INCLUDE */
#define _POSIX_C_SOURCE		200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* DEFINITIONS */
#define SUPER_BLOCK_OFFSET	1024
#define SUPER_BLOCK_SIZE	1024
#define INODE_SIZE		128
#define INODE_BLOCK_POINTER	4
#define GROUP_DESCRIPTOR_TABLE	32

/* STRUCTS */
typedef struct superblock_t {
	uint16_t magic_number;
	uint32_t total_inodes,
		total_blocks,
		block_size,
		fragment_size,
		inodes_per_group,
		blocks_per_group,
		fragments_per_group,
		first_data_block;
} superblock_t;

typedef struct groupdescriptor_t {
	uint16_t number_of_contained_blocks,
		number_of_free_blocks,
		number_of_free_inodes,
		number_of_directories;
	uint32_t inode_bitmap_block,
		block_bitmap_block,
		inode_table_block;
} groupdescriptor_t;

/* GLOBALS */
char 			*disk_image;
superblock_t 		*superblock;
groupdescriptor_t 	*groupdescriptor;
int 			*valid_inodes;
int 			**valid_inode_directories;

int 			total_file_size,
			disk_image_fd,
			super_csv, 
			group_csv,
			bitmap_csv,
			inode_csv,
			directory_csv,
			indirect_csv,
			number_of_groups,
			number_of_valid_inodes = 0;

uint8_t 		buf_8;
uint16_t 		buf_16;
int			ibuf_32;
uint32_t 		buf_32;
uint64_t 		buf_64;

FILE 			*f_super_csv,
			*f_group_csv,
			*f_bitmap_csv,
			*f_directory_csv,
			*f_inode_csv,
			*f_indirect_csv;

/* FUNCTIONS */
int powerOfTwo(uint32_t num) {
	while(num > 1) {
		if(num % 2 == 0)
			num = num / 2;
		else
			return 0;
	}
	return 1;
}

/*
	SUPERBLOCK (9)
	offset 		size 		description
	0		4		s_inodes_count
	4		4		s_blocks_count
	12		4		s_free_blocks_count
	16		4		s_free_inodes_count
	20		4		s_first_data_block
	32		4		s_blocks_per_group
	36		4		s_frags_per_group
	40		4		s_inodes_per_group
	56		2		s_magic
*/
void parseSuperBlock() {
	super_csv = creat("super.csv", S_IRWXU);
	f_super_csv = fopen("super.csv", "w");

	// magic_number
	pread(disk_image_fd, &buf_16, 2, SUPER_BLOCK_OFFSET + 56);
	superblock->magic_number = buf_16;
	fprintf(f_super_csv, "%x,", superblock->magic_number);

	if(superblock->magic_number != 0xef53) {
		fprintf(stderr, "Superblock - invalid magic: %d\n", superblock->magic_number);
		exit(EXIT_FAILURE);
	}

	// total_inodes
	pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET);
	superblock->total_inodes = buf_32;
	fprintf(f_super_csv, "%d,", superblock->total_inodes);

	// total_blocks
	pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + 4);
	superblock->total_blocks = buf_32;
	fprintf(f_super_csv, "%d,", superblock->total_blocks);

	if(superblock->total_blocks > total_file_size) {
		fprintf(stderr, "Superblock - invalid block count %d > image size %d\n", superblock->total_blocks, total_file_size);
		exit(EXIT_FAILURE);
	}

	// block_size
	pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + 24);
	superblock->block_size = SUPER_BLOCK_SIZE << buf_32;
	fprintf(f_super_csv, "%d,", superblock->block_size);

	if(superblock->block_size < 512 || superblock->block_size > 65536 || !powerOfTwo(superblock->block_size)) {
		fprintf(stderr, "Superblock - invalid block size: %d\n", superblock->block_size);
		exit(EXIT_FAILURE);
	}

	// fragmentation_size
	pread(disk_image_fd, &ibuf_32, 4, SUPER_BLOCK_OFFSET + 28);
	if(ibuf_32 > 0) {
		superblock->fragment_size = 1024 << ibuf_32;
	}
	else {
		superblock->fragment_size = 1024 >> -ibuf_32;
	}
	fprintf(f_super_csv, "%d,", superblock->fragment_size);

	// blocks_per_group
	pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + 32);
	superblock->blocks_per_group = buf_32;
	fprintf(f_super_csv, "%d,", superblock->blocks_per_group);

	if(superblock->total_blocks % superblock->blocks_per_group != 0) {
		fprintf(stderr, "Superblock - %d blocks, %d blocks/group", superblock->total_blocks, superblock->blocks_per_group);
		exit(EXIT_FAILURE);
	}

	// inodes_per_group
	pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + 40);
	superblock->inodes_per_group = buf_32;
	fprintf(f_super_csv, "%d,", superblock->inodes_per_group);

	if(superblock->total_inodes % superblock->inodes_per_group != 0) {
		fprintf(stderr, "Superblock - %d Inodes, %d Inodes/group", superblock->total_inodes, superblock->inodes_per_group);
		exit(EXIT_FAILURE);
	}

	//fragments_per_group
	pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + 36);
	superblock->fragments_per_group = buf_32;
	fprintf(f_super_csv, "%d,", superblock->fragments_per_group);

	//first_data_block
	pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + 20);
	superblock->first_data_block = buf_32;
	fprintf(f_super_csv, "%d\n", superblock->first_data_block);

	if(superblock->first_data_block > total_file_size) {
		fprintf(stderr, "Superblock - invalid first block %d > image size %d\n", superblock->first_data_block, total_file_size);
		exit(EXIT_FAILURE);
	}

	fclose(f_super_csv);
	close(super_csv);
}

/*
	GROUP_DESCRIPTOR (7)
	offset 		size 		description
	0		4		bg_block_bitmap
	4		4		bg_inode_bitmap
	8		4		bg_inode_table
	12		2		bg_free_blocks_count
	14		2		bg_free_inodes_count
	16		2		bg_used_dirs_count
*/
void parseGroupDescriptors() {
	group_csv = creat("group.csv", S_IRWXU);
	f_group_csv = fopen("group.csv", "w");

	// Calculate the number of groups
	number_of_groups = ceil((double) superblock->total_blocks / superblock->blocks_per_group);
	double rem = number_of_groups - ((double) superblock->total_blocks / superblock->blocks_per_group);
	groupdescriptor = malloc(number_of_groups * sizeof(groupdescriptor_t));

	if(!groupdescriptor) {
		fprintf(stderr, "ERROR: Malloc failed - groupdescriptor.\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < number_of_groups; i++) {
		// number_of_contained_blocks
		if(i != number_of_groups-1 || rem == 0) {
			groupdescriptor[i].number_of_contained_blocks = superblock->blocks_per_group;
			fprintf(f_group_csv, "%d,", groupdescriptor[i].number_of_contained_blocks);
		}
		else {
			groupdescriptor[i].number_of_contained_blocks = superblock->blocks_per_group * rem;
			fprintf(f_group_csv, "%d,", groupdescriptor[i].number_of_contained_blocks);
		}

		if(groupdescriptor[i].number_of_contained_blocks != superblock->blocks_per_group)
			fprintf(stderr, "Group %d: %d blocks, superblock says %d\n", i+1, groupdescriptor[i].number_of_contained_blocks, superblock->blocks_per_group);

		// number_of_free_blocks
		pread(disk_image_fd, &buf_16, 2, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE) + 12);
		groupdescriptor[i].number_of_free_blocks = buf_16;
		fprintf(f_group_csv, "%d,", groupdescriptor[i].number_of_free_blocks);

		// number_of_free_inodes
		pread(disk_image_fd, &buf_16, 2, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE) + 14);
		groupdescriptor[i].number_of_free_inodes = buf_16;
		fprintf(f_group_csv, "%d,", groupdescriptor[i].number_of_free_inodes);

		// number_of_directories
		pread(disk_image_fd, &buf_16, 2, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE) + 16);
		groupdescriptor[i].number_of_directories = buf_16;
		fprintf(f_group_csv, "%d,", groupdescriptor[i].number_of_directories);

		// (free) inode_bitmap_block
		pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE) + 4);
		groupdescriptor[i].inode_bitmap_block = buf_32;
		fprintf(f_group_csv, "%x,", groupdescriptor[i].inode_bitmap_block);

		if((i + 1) * superblock->blocks_per_group < groupdescriptor[i].inode_bitmap_block || i * superblock->blocks_per_group > groupdescriptor[i].inode_bitmap_block)
			fprintf(stderr, "Group %d: blocks %d-%d, free Inode map starts at %d\n",
				i+1,
				i * superblock->blocks_per_group,
				(i + 1) * superblock->blocks_per_group,
				groupdescriptor[i].inode_bitmap_block);

		// (free) block_bitmap_block
		pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE));
		groupdescriptor[i].block_bitmap_block = buf_32;
		fprintf(f_group_csv, "%x,", groupdescriptor[i].block_bitmap_block);

		if((i + 1) * superblock->blocks_per_group < groupdescriptor[i].block_bitmap_block || i * superblock->blocks_per_group > groupdescriptor[i].block_bitmap_block)
			fprintf(stderr, "Group %d: blocks %d-%d, free block map starts at %d\n",
				i+1,
				i * superblock->blocks_per_group,
				(i + 1) * superblock->blocks_per_group,
				groupdescriptor[i].block_bitmap_block);

		// inode_table_block (start)
		pread(disk_image_fd, &buf_32, 4, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE) + 8);
		groupdescriptor[i].inode_table_block = buf_32;
		fprintf(f_group_csv, "%x\n", groupdescriptor[i].inode_table_block);

		if((i + 1) * superblock->blocks_per_group < groupdescriptor[i].inode_table_block || i * superblock->blocks_per_group > groupdescriptor[i].inode_table_block)
			fprintf(stderr, "Group %d: blocks %d-%d, Inode table starts at %d\n",
				i+1,
				i * superblock->blocks_per_group,
				(i + 1) * superblock->blocks_per_group,
				groupdescriptor[i].inode_table_block);
	}

	fclose(f_group_csv);
	close(group_csv);
}

/*
	"On small file systems, the 'Block Bitmap' is normally located at the 
	first block..."
*/
void parseBitMaps() {
	bitmap_csv = creat("bitmap.csv", S_IRWXU);
	f_bitmap_csv = fopen("bitmap.csv", "w");

	for(int i = 0; i < number_of_groups; i++){
		// Bytes in block bitmap
		for(int j = 0; j < superblock->block_size; j++){
			pread(disk_image_fd, &buf_8, 1, (groupdescriptor[i].block_bitmap_block * superblock->block_size) + j);
			int8_t bit_mask = 1;

			for(int k = 1; k <= 8; k++){
				if((bit_mask & buf_8) == 0){
					fprintf(f_bitmap_csv, "%x,", groupdescriptor[i].block_bitmap_block);
					fprintf(f_bitmap_csv, "%d\n", j * 8 + k + (i * superblock->blocks_per_group));
				}
				bit_mask = bit_mask << 1;
			}
		}

		// Bytes in inode bitmap
		for(int j = 0; j < superblock->block_size; j++){
			pread(disk_image_fd, &buf_8, 1, (groupdescriptor[i].inode_bitmap_block * superblock->block_size) + j);
			int8_t bit_mask = 1;

			for(int k = 1; k <= 8; k++){
				if((bit_mask & buf_8) == 0){
					fprintf(f_bitmap_csv, "%x,", groupdescriptor[i].inode_bitmap_block);
					fprintf(f_bitmap_csv, "%d\n", j * 8 + k + (i * superblock->inodes_per_group));
				}
				bit_mask = bit_mask << 1;
			}
		}
	}

	fclose(f_bitmap_csv);
	close(bitmap_csv);
}


/*
	"The 'Inode Bitmap' works in a similar way as the 'Block Bitmap', 
	difference being in each bit representing an inode in the 'Inode Table'
	rather than a block."

	INODE (12)
	offset 		size 		description
	0		2		i_mode
	2		2		i_uid
	4		4		i_size
	8		4		i_atime
	12		4		i_ctime
	16		4		i_mtime
	24		2		i_gid
	26		2		i_links_count
	28		4		i_blocks
	40		15*4		i_block
	108		4		i_dir_acl
	116		12		i_osd2 (for owner, group)
*/
void parseInodes() {
	inode_csv = creat("inode.csv", S_IRWXU);
	f_inode_csv = fopen("inode.csv", "w");

	valid_inode_directories = malloc(superblock->total_inodes * sizeof(int*));
	valid_inodes = malloc(superblock->total_inodes * sizeof(int));

	if(!valid_inode_directories || !valid_inodes) {
		fprintf(stderr, "ERROR: Malloc failed - valid_inode_directories/valid_inodes.\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < superblock->total_inodes; i++) {
		valid_inode_directories[i] = malloc(2 * sizeof(int));
	}

	for(int i = 0; i < number_of_groups; i++) {
		for(int j = 0; j < superblock->block_size; j++) {
			pread(disk_image_fd, &buf_8, 1, (groupdescriptor[i].inode_bitmap_block * superblock->block_size) + j);
			int8_t bit_mask = 1;

			for(int k = 1; k <= 8; k++) {
				if((bit_mask & buf_8) != 0 && (j * 8 + k ) <= superblock->inodes_per_group) {
					int inode_num = j * 8 + k + (i * superblock->inodes_per_group);
					fprintf(f_inode_csv, "%d,", inode_num);

					// file type
					pread(disk_image_fd, &buf_16, 2, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE);
					valid_inodes[number_of_valid_inodes] = groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE;
					number_of_valid_inodes++;

					if(buf_16 & 0x8000)
						fprintf(f_inode_csv, "f,");
					else if(buf_16 & 0xA000)
						fprintf(f_inode_csv, "s,");
					else if(buf_16 & 0x4000) {
						valid_inode_directories[number_of_valid_inodes][0] = groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE;
						valid_inode_directories[number_of_valid_inodes][1] = inode_num;
						number_of_valid_inodes++;
						fprintf(f_inode_csv, "d,");
					}
					else
						fprintf(f_inode_csv, "?,");

					// mode
					fprintf(f_inode_csv, "%o,", buf_16);

					// owner
					pread(disk_image_fd, &buf_32, 2, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 2);
					uint32_t owner = buf_32;
					pread(disk_image_fd, &buf_32, 2, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 120);
					owner |= buf_32 << 16;
					fprintf(f_inode_csv, "%d,", owner);

					// group
					pread(disk_image_fd, &buf_32, 2, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 24);
					uint32_t group = buf_32;
					pread(disk_image_fd, &buf_32, 2, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 122);
					group |= buf_32 << 16;
					fprintf(f_inode_csv, "%d,", group);

					// link count
					pread(disk_image_fd, &buf_16, 2, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 26);
					fprintf(f_inode_csv, "%d,", buf_16);

					// creation time
					pread(disk_image_fd, &buf_32, 4, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 12);
					fprintf(f_inode_csv, "%x,", buf_32);

					// modification time
					pread(disk_image_fd, &buf_32, 4, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 16);
					fprintf(f_inode_csv, "%x,", buf_32);

					// access time
					pread(disk_image_fd, &buf_32, 4, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 8);
					fprintf(f_inode_csv, "%x,", buf_32);

					// file size
					pread(disk_image_fd, &buf_64, 4, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 4);
					uint64_t file_size = buf_64;
					pread(disk_image_fd, &buf_64, 4, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 108);
					file_size |= buf_64 << 32;
					fprintf(f_inode_csv, "%ld,", file_size);

					// number of blocks
					pread(disk_image_fd, &buf_32, 4, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 28);
					fprintf(f_inode_csv, "%d,", buf_32/(superblock->block_size/512));

					// block pointers
					for(int l = 0; l < 15; l++) {
						pread(disk_image_fd, &buf_32, 4, groupdescriptor[i].inode_table_block * superblock->block_size + (j * 8 + k - 1) * INODE_SIZE + 40 + (l * 4));
						uint32_t val = buf_32;

						if(val > superblock->first_data_block + total_file_size)
							fprintf(stderr, "Inode %d - invalid block pointer[%d]: %d", inode_num, l, val);

						if(l != 14)
							fprintf(f_inode_csv, "%x,", buf_32);
						else
							fprintf(f_inode_csv, "%x\n", buf_32);
					}
					
					buf_16 = 0;
					buf_32 = 0;
					buf_64 = 0;
				}

				bit_mask = bit_mask << 1;
			}
		}
	}

	fclose(f_inode_csv);
	close(inode_csv);
}

/*
	DIRECTORY (6)
	offset 		size 		description
	0		4		inode
	4		2		rec_len
	6		1		name_len[a]
	8		0-255		name
*/
void parseDirectory() {
	directory_csv = creat("directory.csv", S_IRWXU);
	f_directory_csv = fopen("directory.csv", "w");

	for(int i = 0; i < number_of_valid_inodes; i++) {
		int entry = 0;

		// For direct blocks
		for(int j = 0; j < 12; j++) {
			pread(disk_image_fd, &buf_32, 4, (valid_inode_directories[i][0] + 40 + (j * 4)));
			uint32_t offset = buf_32;

			if(offset != 0) {
				int curr_offset = superblock->block_size * offset;

				while(curr_offset < superblock->block_size * offset + superblock->block_size) {
					// name length
					pread(disk_image_fd, &buf_8, 1, curr_offset + 6);
					int name_length = buf_8;

					// inode number
					pread(disk_image_fd, &buf_32, 4, curr_offset);
					int inode_num = buf_32;

					// entry length
					pread(disk_image_fd, &buf_16, 2, curr_offset + 4);
					int entry_length = buf_16;

					if(inode_num == 0) {
						curr_offset += entry_length;
						entry++;
						continue;
					}

					if(entry_length > 1024 || entry_length < 8 || name_length > entry_length) {
						fprintf(stderr, "Inode %d, block %d - bad dirent: len = %d, namelen = %d\n", i+1, curr_offset, entry_length, name_length);
						break;
					}

					if(inode_num > superblock->total_inodes) {
						fprintf(stderr, "Inode %d, block %d - bad dirent: Inode = %d\n", i+1, curr_offset, inode_num);
						break;
					}

					// parent inode
					fprintf(f_directory_csv, "%d,", valid_inode_directories[i][1]);

					// entry
					fprintf(f_directory_csv, "%d,", entry);
					entry++;

					// entry length
					fprintf(f_directory_csv, "%d,", entry_length);

					// name length
					fprintf(f_directory_csv, "%d,", name_length);

					// inode number
					fprintf(f_directory_csv, "%d,", inode_num);

					// name
					char buf_c;
					fprintf(f_directory_csv, "\"");

					for(int k = 0; k < name_length; k++) {
						pread(disk_image_fd, &buf_c, 1, curr_offset + 8 + k);
						fprintf(f_directory_csv, "%c", buf_c);
					}
					fprintf(f_directory_csv, "\"\n");

					curr_offset += entry_length;
				}
			}
		}

		// For indirect blocks
		pread(disk_image_fd, &buf_32, 4, (valid_inode_directories[i][0] + 88));
		uint32_t offset = buf_32;

		if(offset != 0) {
			for(int j = 0; j < superblock->block_size / 4; j++) {
				int curr_offset = superblock->block_size * offset + (j * 4);
				pread(disk_image_fd, &buf_32, 4, curr_offset);
				int block_num = buf_32;

				if(block_num != 0) {
					curr_offset = block_num * superblock->block_size;
					while(curr_offset < block_num * superblock->block_size + superblock->block_size) {
						// name length
						pread(disk_image_fd, &buf_8, 1, curr_offset + 6);
						int name_length = buf_8;

						// inode number
						pread(disk_image_fd, &buf_32, 4, curr_offset);
						int inode_num = buf_32;

						// entry length
						pread(disk_image_fd, &buf_16, 2, curr_offset + 4);
						int entry_length = buf_16;

						if(inode_num == 0) {
							curr_offset += entry_length;
							entry++;
							continue;
						}

						// parent inode
						fprintf(f_directory_csv, "%d,", valid_inode_directories[i][1]);

						// entry
						fprintf(f_directory_csv, "%d,", entry);
						entry++;

						// entry length
						fprintf(f_directory_csv, "%d,", entry_length);

						// name length
						fprintf(f_directory_csv, "%d,", name_length);

						// inode number
						fprintf(f_directory_csv, "%d,", inode_num);

						// name
						char buf_c;
						fprintf(f_directory_csv, "\"");

						for(int k = 0; k < name_length; k++) {
							pread(disk_image_fd, &buf_c, 1, curr_offset + 8 + k);
							fprintf(f_directory_csv, "%c", buf_c);
						}
						fprintf(f_directory_csv, "\"\n");

						curr_offset += entry_length;
					}
				}
			}
		}

		// For double indirect blocks
		pread(disk_image_fd, &buf_32, 4, (valid_inode_directories[i][0] + 92));
		offset = buf_32;

		if(offset != 0) {
			for(int j = 0; j < superblock->block_size / 4; j++) {
				int curr_offset = superblock->block_size * offset + (j * 4);
				pread(disk_image_fd, &buf_32, 4, curr_offset);
				int block_num = buf_32;

				if(block_num != 0) {
					for(int k = 0; k < superblock->block_size / 4; k++) {
						pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (k * 4));
						int block_num2 = buf_32;
						if(block_num2 != 0) {
							curr_offset = block_num2 * superblock->block_size;
							while(curr_offset < block_num2 * superblock->block_size + superblock->block_size) {
								// name length
								pread(disk_image_fd, &buf_8, 1, curr_offset + 6);
								int name_length = buf_8;

								// inode number
								pread(disk_image_fd, &buf_32, 4, curr_offset);
								int inode_num = buf_32;

								// entry length
								pread(disk_image_fd, &buf_16, 2, curr_offset + 4);
								int entry_length = buf_16;

								if(inode_num == 0) {
									curr_offset += entry_length;
									entry++;
									continue;
								}

								// parent inode
								fprintf(f_directory_csv, "%d,", valid_inode_directories[i][1]);

								// entry
								fprintf(f_directory_csv, "%d,", entry);
								entry++;

								// entry length
								fprintf(f_directory_csv, "%d,", entry_length);

								// name length
								fprintf(f_directory_csv, "%d,", name_length);

								// inode number
								fprintf(f_directory_csv, "%d,", inode_num);

								// name
								char buf_c;
								fprintf(f_directory_csv, "\"");

								for(int k = 0; k < name_length; k++) {
									pread(disk_image_fd, &buf_c, 1, curr_offset + 8 + k);
									fprintf(f_directory_csv, "%c", buf_c);
								}
								fprintf(f_directory_csv, "\"\n");

								curr_offset += entry_length;
							}
						}
					}
				}
			}
		}

		// For triple indirect blocks
		pread(disk_image_fd, &buf_32, 4, (valid_inode_directories[i][0] + 96));
		offset = buf_32;

		if(offset != 0) {
			for(int j = 0; j < superblock->block_size / 4; j++) {
				int curr_offset = superblock->block_size * offset + (j * 4);
				pread(disk_image_fd, &buf_32, 4, curr_offset);
				int block_num = buf_32;

				if(block_num != 0) {
					for(int k = 0; k < superblock->block_size / 4; k++) {
						pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (k * 4));
						int block_num2 = buf_32;

						if(block_num2 != 0) {
							for(int l = 0; l < superblock->block_size / 4; l++) {
								pread(disk_image_fd, &buf_32, 4, block_num2 * superblock->block_size + (l * 4));
								int block_num3 = buf_32;

								if(block_num3 != 0) {
									curr_offset = block_num3 * superblock->block_size;
									while(curr_offset < block_num3 * superblock->block_size + superblock->block_size) {
										// name length
										pread(disk_image_fd, &buf_8, 1, curr_offset + 6);
										int name_length = buf_8;

										// inode number
										pread(disk_image_fd, &buf_32, 4, curr_offset);
										int inode_num = buf_32;

										// entry length
										pread(disk_image_fd, &buf_16, 2, curr_offset + 4);
										int entry_length = buf_16;

										if(inode_num == 0) {
											curr_offset += entry_length;
											entry++;
											continue;
										}

										// parent inode
										fprintf(f_directory_csv, "%d,", valid_inode_directories[i][1]);

										// entry
										fprintf(f_directory_csv, "%d,", entry);
										entry++;

										// entry length
										fprintf(f_directory_csv, "%d,", entry_length);

										// name length
										fprintf(f_directory_csv, "%d,", name_length);

										// inode number
										fprintf(f_directory_csv, "%d,", inode_num);

										// name
										char buf_c;
										fprintf(f_directory_csv, "\"");

										for(int k = 0; k < name_length; k++) {
											pread(disk_image_fd, &buf_c, 1, curr_offset + 8 + k);
											fprintf(f_directory_csv, "%c", buf_c);
										}
										fprintf(f_directory_csv, "\"\n");

										curr_offset += entry_length;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	fclose(f_directory_csv);
	close(directory_csv);
}

void parseIndirectBlock() {
	indirect_csv = creat("indirect.csv", S_IRWXU);
	f_indirect_csv = fopen("indirect.csv", "w");

	int entry = 0;

	for(int i = 0; i < number_of_valid_inodes; i++) {
		// For a single indirect block
		entry = 0;
		pread(disk_image_fd, &buf_32, 4, valid_inodes[i] + 88);
		int block_num = buf_32;

		for(int j = 0; j < superblock->block_size / 4; j++) {
			pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (j * 4));
			int block_num2 = buf_32;

			if(block_num2 > superblock->total_blocks) {
				fprintf(stderr, "Indirect block %x - invalid entry[%d] = %x", block_num, i, block_num2);
			}

			if(block_num2 != 0) {
				fprintf(f_indirect_csv, "%x,", block_num);
				fprintf(f_indirect_csv, "%d,", entry);
				fprintf(f_indirect_csv, "%x\n", block_num2);
				entry++;
			}
		}

		// For a double indirect block
		entry = 0;
		pread(disk_image_fd, &buf_32, 4, valid_inodes[i] + 92);
		block_num = buf_32;

		for(int j = 0; j < superblock->block_size / 4; j++) {
			pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (j * 4));
			int block_num2 = buf_32;

			if(block_num2 != 0) {
				//fprintf(f_indirect_csv, "%x,", block_num);
				//fprintf(f_indirect_csv, "%d,", entry);
				//fprintf(f_indirect_csv, "%x\n", block_num2);
				entry++;
			}
		}

		entry = 0;

		for(int j = 0; j < superblock->block_size / 4; j++) {
			pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (j * 4));
			int block_num2 = buf_32;

			if(block_num2 != 0) {
				entry = 0;

				for(int k = 0; k < superblock->block_size / 4; k++) {
					pread(disk_image_fd, &buf_32, 4, block_num2 * superblock->block_size + (k * 4));
					int block_num3 = buf_32;

					if(block_num3 != 0) {
						//fprintf(f_indirect_csv, "%x,", block_num2);
						//fprintf(f_indirect_csv, "%d,", entry);
						//fprintf(f_indirect_csv, "%x\n", block_num3);
						entry++;
					}
				}
			}
		}

		// For a triple indirect block
		entry = 0;
		pread(disk_image_fd, &buf_32, 4, valid_inodes[i] + 96);
		block_num = buf_32;

		for(int j = 0; j < superblock->block_size / 4; j++) {
			pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (j * 4));
			int block_num2 = buf_32;

			if(block_num2 != 0) {
				//fprintf(f_indirect_csv, "%x,", block_num);
				//fprintf(f_indirect_csv, "%d,", entry);
				//fprintf(f_indirect_csv, "%x\n", block_num2);
				entry++;
			}
		}

		entry = 0;

		for(int j = 0; j < superblock->block_size / 4; j++) {
			pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (j * 4));
			int block_num2 = buf_32;

			if(block_num2 != 0) {
				entry = 0;

				for(int k = 0; k < superblock->block_size / 4; k++) {
					pread(disk_image_fd, &buf_32, 4, block_num2 * superblock->block_size + (k * 4));
					int block_num3 = buf_32;

					if(block_num3 != 0) {
						//fprintf(f_indirect_csv, "%x,", block_num2);
						//fprintf(f_indirect_csv, "%d,", entry);
						//fprintf(f_indirect_csv, "%x\n", block_num3);
						entry++;
					}
				}
			}
		}

		entry = 0;

		for(int j = 0; j < superblock->block_size / 4; j++) {
			pread(disk_image_fd, &buf_32, 4, block_num * superblock->block_size + (j * 4));
			int block_num2 = buf_32;

			if(block_num2 != 0) {
				entry = 0;

				for(int k = 0; k < superblock->block_size / 4; k++) {
					pread(disk_image_fd, &buf_32, 4, block_num2 * superblock->block_size + (k * 4));
					int block_num3 = buf_32;

					if(block_num3 != 0) {
						entry = 0;

						for(int l = 0; l < superblock->block_size / 4; l++) {
							pread(disk_image_fd, &buf_32, 4, block_num3 * superblock->block_size + (l * 4));
							int block_num4 = buf_32;

							if(block_num4 != 0) {
								//fprintf(f_indirect_csv, "%x,", block_num3);
								//fprintf(f_indirect_csv, "%d,", entry);
								//fprintf(f_indirect_csv, "%x\n", block_num4);
								entry++;
							}
						}
					}
				}
			}
		}

	}

	fclose(f_indirect_csv);
	close(indirect_csv);
}

/* MAIN */
int main(int argc, char **argv) {
	/* Check to see if the one and only argument is the disk-image! */
	if(argc != 2) {
		fprintf(stderr, "ERROR: You should have exactly 1 argument (disk_image).\n");
		exit(EXIT_FAILURE);
	}
	else {
		struct stat sb;
		if(stat(argv[1], &sb) == -1) {
			fprintf(stderr, "ERROR: Stat failed.\n");
			exit(EXIT_FAILURE);
		}
		total_file_size = sb.st_size;

		disk_image = malloc((strlen(argv[1]) + 1) * sizeof(char));
		superblock = malloc(sizeof(superblock_t));

		if(!disk_image || !superblock) {
			fprintf(stderr, "ERROR: Malloc failed - disk_image/superblock.\n");
			exit(EXIT_FAILURE);
		}

		disk_image = argv[1];
		disk_image_fd = open(disk_image, O_RDONLY);
	}

	parseSuperBlock();
	parseGroupDescriptors();
	parseBitMaps();
	parseInodes();
	parseDirectory();
	parseIndirectBlock();

	close(disk_image_fd);
	exit(EXIT_SUCCESS);
}