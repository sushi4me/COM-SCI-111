from optparse 	import OptionParser

import csv
import sys

# GLOBALS
BITMAP_FREE_BLOCKS 	= []
FREE_BLOCKS 		= []
INVALID_BLOCKS		= []
BITMAP_FREE_INODES 	= []
FREE_INODES 		= []
INVALID_DIR_ENTRY 	= []

IN_USE_BLOCKS 		= {}
IN_USE_INODES 		= {}
IN_USE_DIR 		= {}
INDIRECT_BLOCKS 	= {}
UNALLOCATED_INODE 	= {}

# CLASS DECLARATIONS
class Block():
	def __init__(self, blocks, inodes, entry, indirect_block):
		self.mBlockEntry 	= blocks
		self.mInodeEntry 	= inodes
		self.mEntryNum 		= entry
		self.mIndirectBlock 	= indirect_block
		self.mReferencePointers = []

class Inode():
	def __init__(self, inode, link):
		self.mInodeEntry	= inode
		self.mLinkCount		= link
		self.mDirEntries	= []
		self.mBlockPointers	= []

class Directory():
	def __init__(self, inode, parent, entry, entry_name):
		self.mInodeEntry 	= inode
		self.mParentEntry	= parent
		self.mEntryNum		= entry
		self.mEntryName		= entry_name

# FUNCTIONS
def getFileSystemData():
	# Open the files that we need
	super_csv = csv.reader(open('super.csv', 'rb'), 
		delimiter=',', 
		quotechar='"')
	group_csv = csv.reader(open('group.csv', 'rb'), 
		delimiter=',', 
		quotechar='"')
	bitmap_csv = csv.reader(open('bitmap.csv', 'rb'),
		delimiter=',')

	# Parse data for super.csv
	for line in super_csv:
		global MAGIC_NUMBER
		global TOTAL_BLOCKS
		global TOTAL_INODES
		global BLOCK_SIZE
		global FRAGMENT_SIZE
		global BLOCKS_PER_GROUP
		global INODES_PER_GROUP
		global FRAGMENTS_PER_GROUP
		global FIRST_DATA_BLOCK

		MAGIC_NUMBER		= int(line[0], 16)	# Convert from hex
		TOTAL_INODES		= int(line[1])
		TOTAL_BLOCKS		= int(line[2])
		BLOCK_SIZE		= int(line[3])
		FRAGMENT_SIZE		= int(line[4])
		BLOCKS_PER_GROUP	= int(line[5])
		INODES_PER_GROUP	= int(line[6])
		FRAGMENTS_PER_GROUP	= int(line[7])
		FIRST_DATA_BLOCK	= int(line[8])

	if DEBUGGING:
		print "SUPERBLOCK: %d, %d, %d, %d, %d, %d, %d, %d, %d" % (MAGIC_NUMBER, 
			TOTAL_INODES, 
			TOTAL_BLOCKS,
			BLOCK_SIZE,
			FRAGMENT_SIZE,
			BLOCKS_PER_GROUP,
			INODES_PER_GROUP,
			FRAGMENTS_PER_GROUP,
			FIRST_DATA_BLOCK)

	# Parse data for group.csv
	for line in group_csv:
		global IN_USE_BLOCKS
		global BITMAP_FREE_BLOCKS
		global BITMAP_FREE_INODES

		free_block_bitmap = int(line[5], 16)
		free_inode_bitmap = int(line[4], 16)
		BITMAP_FREE_BLOCKS.append(free_block_bitmap)
		BITMAP_FREE_INODES.append(free_inode_bitmap)
		IN_USE_BLOCKS[free_block_bitmap] = Block(free_block_bitmap, 0, 0, 0)
		IN_USE_BLOCKS[free_inode_bitmap] = Block(free_inode_bitmap, 0, 0, 0)

	# Parse data for bitmap.csv
	for line in bitmap_csv:
		global FREE_BLOCKS
		global FREE_INODES

		free_number = int(line[1])
		map_number = int(line[0], 16)

		if map_number in BITMAP_FREE_BLOCKS:
			FREE_BLOCKS.append(free_number)
		elif map_number in BITMAP_FREE_INODES:
			FREE_INODES.append(free_number)

	return

def append_IN_USE_BLOCKS(block_num, inode_num, entry, indirect_num):
	global TOTAL_BLOCKS, IN_USE_BLOCKS, INVALID_BLOCKS

	if block_num == 0 or block_num > TOTAL_BLOCKS:
		INVALID_BLOCKS.append((block_num, inode_num, indirect_num, entry))
	else:
		if block_num not in IN_USE_BLOCKS:
			IN_USE_BLOCKS[block_num] = Block(block_num, inode_num, entry, indirect_num)
		IN_USE_BLOCKS[block_num].mReferencePointers.append((inode_num, indirect_num, entry))

	return

def getSingleBlock(current, inode, indirect, entry):
	global INDIRECT_BLOCKS
	count = 1
	append_IN_USE_BLOCKS(current, inode, entry, indirect)
	if current in INDIRECT_BLOCKS:
		for line in INDIRECT_BLOCKS[current]:
			count = count + 1
			append_IN_USE_BLOCKS(line[1], inode, line[0], current)
	return count

def getDoubleBlock(current, inode, indirect, entry):
	global INDIRECT_BLOCKS
	count = 1
	append_IN_USE_BLOCKS(current, inode, entry, indirect)
	if current in INDIRECT_BLOCKS:
		for line in INDIRECT_BLOCKS[current]:
			count = count + getSingleBlock(line[1], inode, current, line[0])
	return count

def getTripleBlock(current, inode, indirect, entry):
	global INDIRECT_BLOCKS
	count = 1
	append_IN_USE_BLOCKS(current, inode, entry, indirect)
	if current in INDIRECT_BLOCKS:
		for line in INDIRECT_BLOCKS[current]:
			count = count + getDoubleBlock(line[1], inode, current, line[0])
	return count

def checkInodes():
	global TOTAL_BLOCKS, INVALID_BLOCKS, IN_USE_INODES

	inode_csv = csv.reader(open('inode.csv', 'rb'),
		delimiter=',')

	for line in inode_csv:
		block_pointers = []
		inode = int(line[0])
		link_count = int(line[5])
		num_blocks = int(line[10])
		pending = num_blocks - 12
		IN_USE_INODES[inode] = Inode(inode, link_count)

		# Check validity of direct blocks
		for ptr in range(11, 11 + min(12, num_blocks)):
			append_IN_USE_BLOCKS(int(line[ptr], 16), inode, ptr - 11, 0)

		# Go through indirect blocks - single, double, triple
		for i in range(1, 4):
			if pending > 0:
				current = int(line[i + 22], 16)
				if current == 0 or current > TOTAL_BLOCKS:
					INVALID_BLOCKS.append((current, inode, 0, i + 11))
				else:
					if i == 1: blocksInsideBlock = getSingleBlock(current, inode, 0, i + 11)
					elif i == 2: blocksInsideBlock = getDoubleBlock(current, inode, 0, i + 11)
					elif i == 3: blocksInsideBlock = getTripleBlock(current, inode, 0, i + 11)
					pending = pending - blocksInsideBlock
	return

def checkIndirect():
	global INDIRECT_BLOCKS

	indirect_csv = csv.reader(open('indirect.csv', 'rb'),
		delimiter=',')

	for line in indirect_csv:
		contains 	= int(line[0], 16)
		entry_num 	= int(line[1])
		block_pointer	= int(line[2], 16)

		if contains not in INDIRECT_BLOCKS:
			INDIRECT_BLOCKS[contains] = [(entry_num, block_pointer)]
		else:
			INDIRECT_BLOCKS[contains].append((entry_num, block_pointer))

	return

def checkDirectories():
	global IN_USE_INODES, UNALLOCATED_INODE, IN_USE_DIR, INVALID_DIR_ENTRY

	directory_csv = csv.reader(open('directory.csv', 'rb'), delimiter=",")

	for line in directory_csv:
		# Give correct typing to current otherwise 229 fails
		current = Directory(int(line[4]), int(line[0]), int(line[1]), str(line[5]))
		error = (current.mParentEntry, current.mEntryNum)

		if current.mParentEntry == 2 or current.mInodeEntry != current.mParentEntry:
			IN_USE_DIR[current.mInodeEntry] = current

		if current.mInodeEntry in IN_USE_INODES:
			IN_USE_INODES[current.mInodeEntry].mDirEntries.append(error)
		elif current.mInodeEntry in UNALLOCATED_INODE:
			UNALLOCATED_INODE[current.mInodeEntry].append(error)
		else:
			UNALLOCATED_INODE[current.mInodeEntry] = [error]

		parent = None
		if (current.mEntryNum == 0 or current.mEntryName == '.') and \
		current.mInodeEntry != current.mParentEntry:
			parent = current.mParentEntry
		elif (current.mEntryNum == 1 or current.mEntryName == '..') and \
		(current.mInodeEntry != IN_USE_DIR[current.mParentEntry].mParentEntry or current.mParentEntry not in IN_USE_DIR):
			parent = IN_USE_DIR[current.mParentEntry].mParentEntry

		if parent is not None:
			INVALID_DIR_ENTRY.append((current.mParentEntry, current.mEntryName, current.mInodeEntry, parent))

	return

def writeToOutput(lab3b_check_txt):
	global BITMAP_FREE_BLOCKS
	global FREE_BLOCKS
	global INVALID_BLOCKS
	global BITMAP_FREE_INODES
	global FREE_INODES
	global INVALID_DIR_ENTRY
	global IN_USE_BLOCKS
	global IN_USE_INODES
	global IN_USE_DIR
	global INDIRECT_BLOCKS
	global UNALLOCATED_INODE

	# strip() deletes beginning and trailing spaces

	# Unallcoated block referenced by inode
	for obj in IN_USE_BLOCKS:
		if obj in FREE_BLOCKS:
			line = "UNALLOCATED BLOCK < "	\
				+ str(obj)		\
				+ " > REFERENCED BY "	\
				+ "INODE < "
			for (inode, indirect, entry) in IN_USE_BLOCKS[obj].mReferencePointers:
				if indirect != 0:
					line = line + str(inode) 	\
						+ " > INDIRECT BLOCK < "\
						+ str(indirect) 	\
						+ " > ENTRY < " 	\
						+ str(entry) 		\
						+ " >"
				else:
					line = line + str(inode)\
						+ " > ENTRY < " \
						+ str(entry) 	\
						+ " >"
			lab3b_check_txt.write(line.strip() + "\n")

	# Duplicated blocks
	for obj in IN_USE_BLOCKS:
		if len(IN_USE_BLOCKS[obj].mReferencePointers) > 1:
			line = "MULTIPLY REFERENCED BLOCK < "	\
				+ str(obj)			\
				+ " > BY "	
			for (inode, indirect, entry) in IN_USE_BLOCKS[obj].mReferencePointers:
				if indirect != 0:
					line = line + "INODE < "  \
						+ str(inode)		\
						+ " > INDIRECT BLOCK < "\
						+ str(indirect)		\
						+ " > ENTRY < "		\
						+ str(entry)		\
						+ " > "
				else:
					line = line + "INODE < "	\
						+ str(inode)		\
						+ " > ENTRY < "		\
						+ str(entry)		\
						+ " > "
			lab3b_check_txt.write(line.strip() + "\n")

	# Unallocated inode
	for obj in UNALLOCATED_INODE:
		line = "UNALLOCATED INODE < "	\
			+ str(obj)		\
			+ " REFERENCED BY "
		for contains in UNALLOCATED_INODE[obj]:
			line = line + "DIRECTORY < "	\
				+ str(contains[0])	\
				+ " > ENTRY < "		\
				+ str(contains[1])	\
				+ " > "
		lab3b_check_txt.write(line.strip() + "\n")
	
	# Invalid link
	line = ""
	for obj in IN_USE_INODES:
		inode = IN_USE_INODES[obj]
		links = len(inode.mDirEntries)

		if obj > 10 and links == 0:
			line = line + "MISSING INODE < "			\
				+ str(obj)					\
				+" > SHOULD BE IN FREE LIST < "			\
				+ str(BITMAP_FREE_INODES[obj/INODES_PER_GROUP])	\
				+ " >\n"
		elif links != inode.mLinkCount:
			line = line + "LINKCOUNT < " 	\
				+ str(obj)		\
				+ " > IS < "		\
				+ str(inode.mLinkCount)	\
				+ " > SHOULD BE < "	\
				+ str(links)		\
				+ " >\n"
	lab3b_check_txt.write(line)

	# Invalid directory
	for (invalid_parent, entry, inode, correct_parent) in INVALID_DIR_ENTRY:
		line = "INCORRECT ENTRY IN < "		\
			+ str(invalid_parent)		\
			+ " > NAME < "			\
			+ str(entry)			\
			+ " > LINK TO < "		\
			+ str(inode)			\
			+ " > SHOULD BE < "		\
			+ str(correct_parent)		\
			+ " >"
		lab3b_check_txt.write(line.strip() + "\n")

	# Invalid block
	for (block, inode, indirect, entry) in INVALID_BLOCKS:
		line = "INVALID BLOCK < "	\
			+ str(block)		\
			+ " > IN INODE < "	\
			+ str(inode)		\
			+ " > "
		if indirect != 0:
			line = line + " INDIRECT BLOCK < "	\
				+ str(indirect)			\
				+ " > "
		line = line + "ENTRY < "	\
			+ str(entry)		\
			+ " >\n"
		lab3b_check_txt.write(line.strip() + "\n")

	return

# MAIN
def main():
	# Globals
	global DEBUGGING

	# Option parser
	version_msg = "lab3b.py 1.0"
	usage_msg = "Reports incorrect blocks/inodes in file system."

	# Add option for debugging
	parser = OptionParser(version=version_msg, usage=usage_msg)
	parser.add_option("-d", "--debugging", 
		action="store_true", 
		dest="debugging", 
		default=False)
	options, args = parser.parse_args(sys.argv[1:])
	DEBUGGING = options.debugging

	# Open file
	lab3b_check_txt = open('lab3b_check.txt', 'w')

	# Run helper functions here
	getFileSystemData()
	checkIndirect()
	checkInodes()
	checkDirectories()
	writeToOutput(lab3b_check_txt)

	# Close file
	lab3b_check_txt.close()

if __name__ == "__main__":
	main()