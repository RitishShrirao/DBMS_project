#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pds_sample.h"

struct PDS_RepoInfo repo_handle;

// pds_create
// Open the data file and index file in "wb" mode
// Initialize index file by storing "0" to indicate there are zero entries in index file
// close the files
int pds_create(char *repo_name, char *linked_repo_name){
	// Create repo data file
	char filename[strlen(repo_name) + 5];
	strcpy(filename, repo_name);
	strcat(filename, ".dat");
	FILE * fp = fopen(filename, "wb+");

	// Create repo index file
	char ndx_file_name[strlen(repo_name) + 5];
	strcpy(ndx_file_name, repo_name);
	strcat(ndx_file_name, ".ndx");
	FILE * fp2 = fopen(ndx_file_name, "wb+");
	if(fp == NULL || fp2 == NULL){
		return PDS_FILE_ERROR;
	}

	// Zero entry in index file
	int zero = 0;
	fwrite(&zero, sizeof(int), 1, fp2);

	int close_status = fclose(fp);
	if (close_status != 0)	return PDS_FILE_ERROR;
	close_status = fclose(fp2);
	if (close_status != 0)	return PDS_FILE_ERROR;

	if (strcmp(linked_repo_name, "NULL") != 0)
	{
		// Create empty linked repo data file
		char linked_file_name[strlen(linked_repo_name) + 5];
		strcpy(linked_file_name, linked_repo_name);
		strcat(linked_file_name, ".dat");
		FILE * fp3 = fopen(linked_file_name, "wb+");

		// Create link data file
		char link_data_file_name[strlen(repo_name) + 12];
		strcpy(link_data_file_name, repo_name);
		strcat(link_data_file_name, "_link.dat");
		FILE * fp4 = fopen(link_data_file_name, "wb+");

		// Check for NULL pointers
		if (fp3 == NULL || fp4 == NULL){
			return PDS_FILE_ERROR;
		}

		// Closing file and checking close status
		close_status = fclose(fp3);
		if (close_status != 0)	return PDS_FILE_ERROR;
		close_status = fclose(fp4);
		if (close_status != 0)	return PDS_FILE_ERROR;
	}

	return PDS_SUCCESS;
}

// pds_open
// Open the data file and index file in rb+ mode
// Update the fields of PDS_RepoInfo appropriately
// Read the number of records form the index file
// Load the index into the array and store in ndx_root by reading index entries from the index file
// Close only the index file
int pds_open( char *repo_name, char * linked_repo_name, int rec_size, int linked_rec_size){
	char filename[strlen(repo_name) + 5];
	char ndx_file_name[strlen(repo_name) + 5];
	char linked_file_name[strlen(repo_name) + 5];
	char link_data_file_name[strlen(repo_name) + 12];
	strcpy(filename, repo_name);
	strcat(filename, ".dat");
	strcpy(ndx_file_name, repo_name);
	strcat(ndx_file_name, ".ndx");

	FILE * fp = fopen(filename, "rb+");
	FILE * fp2 = fopen(ndx_file_name, "rb");
	FILE * fp3 = NULL;
	FILE * fp4 = NULL;

	if(fp == NULL || fp2 == NULL){
		return PDS_FILE_ERROR;
	}

	if (strcmp(linked_repo_name, "NULL") != 0)
	{
		strcpy(linked_file_name, linked_repo_name);
		strcat(linked_file_name, ".dat");
		strcpy(link_data_file_name, repo_name);
		strcat(link_data_file_name, "_link.dat");

		fp3 = fopen(linked_file_name, "rb+");
		fp4 = fopen(link_data_file_name, "rb+");

		if (fp3 == NULL || fp4 == NULL){
			return PDS_FILE_ERROR;
		}
	}

	// Read the number of records
	int num_records;
	fread(&num_records, sizeof(int), 1, fp2);

	// Update the struct PDS_RepoInfo
	repo_handle.pds_data_fp = fp;
	repo_handle.pds_ndx_fp = fp2;
	strcpy(repo_handle.pds_name, repo_name);
	repo_handle.rec_size = rec_size;
	repo_handle.rec_count = num_records;
	repo_handle.repo_status = PDS_REPO_OPEN;
	repo_handle.pds_linked_data_fp = fp3;
	repo_handle.pds_link_fp = fp4;
	repo_handle.linked_rec_size = linked_rec_size;

	// Load the index into the BST
	int load_status = pds_load_ndx();
	if (load_status != PDS_SUCCESS)	return PDS_FILE_ERROR;
	
	int close_status = fclose(fp2);	// Close index file only
	if (close_status != 0)	return PDS_FILE_ERROR;
	
	return PDS_SUCCESS;
}

// pds_load_ndx - Internal function
// Load the index entries into the BST ndx_root by calling bst_add_node repeatedly for each 
// index entry. Unlike array, for BST, you need read the index file one by one in a loop
int pds_load_ndx(){
	// Load the index into the BST using loop and bst_add_node
	repo_handle.ndx_root = NULL;
	for (int i = 0; i < repo_handle.rec_count; i++){
		// Read ndx_entry from file
		struct PDS_NdxInfo *ndx_entry = malloc(sizeof(struct PDS_NdxInfo));
		fread(ndx_entry, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp);

		// Add entry to BST
		bst_add_node(&repo_handle.ndx_root, ndx_entry->key, ndx_entry);
	}
	// bst_print(repo_handle.ndx_root);
	return PDS_SUCCESS;
}

// put_rec_by_key
// Seek to the end of the data file
// Create an index entry with the current data file location using ftell
// Add index entry to BST by calling bst_add_node
// Increment record count
// Write the record at the end of the file
// Return failure in case of duplicate key
int put_rec_by_key( int key, void *rec ){
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}

	// Seek to the end of the data file
	FILE * fp = repo_handle.pds_data_fp;
	if(fp == NULL){
		return PDS_FILE_ERROR;
	}
	
	// Search for key in BST
	struct BST_Node *i = bst_search(repo_handle.ndx_root, key);

	// Key already present in BST
	if (i != NULL)
	{
		struct PDS_NdxInfo *tmp = i->data;
		if (tmp->is_deleted == 0)
		{
			// Key not deleted
			return PDS_ADD_FAILED;
		}
		else
		{
			// Overwrite the record if key was deleted
			tmp->is_deleted = 0;
			int fseek_status = fseek(fp, tmp->offset + sizeof(int), SEEK_SET);
			if (fseek_status != 0)	return PDS_FILE_ERROR;

			fwrite(rec, repo_handle.rec_size, 1, fp);

			return PDS_SUCCESS;
		}
	}

	// Add new record at the end of the file
	int seek_status = fseek(fp, 0, SEEK_END);
	if (seek_status != 0)	return PDS_FILE_ERROR;

	// Add index entry to the BST using offset returned by ftell
	struct PDS_NdxInfo *ndx_entry = malloc(sizeof(struct PDS_NdxInfo));

	if (ndx_entry == NULL)	return PDS_MALLOC_FAILED;

	ndx_entry->key = key;
	ndx_entry->offset = ftell(fp);
	ndx_entry->is_deleted = 0;

	// Add entry to BST
	bst_add_node(&repo_handle.ndx_root, key, ndx_entry);
	repo_handle.rec_count++;		// Increment rec_count

	// Write the record at the end of the file
	fwrite(&key, sizeof(int), 1, fp);
	fwrite(rec, repo_handle.rec_size, 1, fp);
	fwrite(&(ndx_entry->is_deleted), sizeof(int), 1, fp);

	return PDS_SUCCESS;
}

int put_linked_rec_by_key( int key, void * rec)
{
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}
	// Seek to the end of the linked data file
	FILE * fp = repo_handle.pds_linked_data_fp;
	if(fp == NULL){
		return PDS_FILE_ERROR;
	}

	// Add new record at the end of the
	fseek(fp, 0, SEEK_END);

	// Write the record at the end of the file
	fwrite(&key, sizeof(int), 1, fp);
	fwrite(rec, repo_handle.linked_rec_size, 1, fp);

	return PDS_SUCCESS;
}

// get_rec_by_ndx_key
// Search for index entry in BST by calling bst_search
// Seek to the file location based on offset in index entry
// Read the record from the current location
int get_rec_by_ndx_key( int key, void *rec ){
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}
	// Search for index entry in ndx_array
	struct BST_Node *i = bst_search(repo_handle.ndx_root, key);

	// bst_print(repo_handle.ndx_root);
	
	// Key not found
	if (i == NULL){
		return PDS_REC_NOT_FOUND;
	}
	
	// If record is_deleted
	if (((struct PDS_NdxInfo *) (i->data))->is_deleted == 1){
		return PDS_REC_NOT_FOUND;
	}

	// Seek to the file location based on offset in index entry
	FILE * fp = repo_handle.pds_data_fp;
	if(fp == NULL){
		return PDS_FILE_ERROR;
	}
	int offset = (((struct PDS_NdxInfo *) (i->data))->offset);
	fseek(fp, offset, SEEK_SET);

	// Read the key and record from the current location
	int key_read;
	fread(&key_read, sizeof(int), 1, fp);
	fread(rec, repo_handle.rec_size, 1, fp);
	return PDS_SUCCESS;
}

// Brute-force retrieval using an arbitrary search key
// io_count = 0
// Loop through data file till EOF
//	fread the key and record 
//	io_count++
//      Invoke the matcher with the current record and input non-ndx search key
//	if mathcher returns success, return the current record, else continue the loop
// end loop
int get_rec_by_non_ndx_key(void *non_ndx_key, void *rec, int (*matcher)(void *rec, void *non_ndx_key), int *io_count){
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}

	// Loop through data file till EOF
	FILE * fp = repo_handle.pds_data_fp;
	if(fp == NULL){
		return PDS_FILE_ERROR;
	}
	fseek(fp, 0, SEEK_SET);
	*io_count = 0;
	while(1){
		// Read the key and record
		int key_read, is_deleted;
		fread(&key_read, sizeof(int), 1, fp);

		// Break when EOF
		if(feof(fp)){
			break;
		}

		// Read record and increment io_count
		fread(rec, repo_handle.rec_size, 1, fp);
		(*io_count)++;

		fread(&is_deleted, sizeof(int), 1, fp);

		// Check match using matcher function
		if(matcher(rec, non_ndx_key) == 0){
			if (is_deleted)	return PDS_REC_NOT_FOUND;
			return PDS_SUCCESS;
		}
	}

	// Return failure if record not found
	*io_count = -1;
	return PDS_REC_NOT_FOUND;
}

int get_linked_rec_by_key(int key, void * rec, int * io_count)
{
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}
	
	// Loop through data file till EOF
	FILE * fp = repo_handle.pds_linked_data_fp;
	if(fp == NULL){
		return PDS_FILE_ERROR;
	}

	// Seek to the start of the file
	int lp_set = fseek(fp, 0, SEEK_SET);
	if (lp_set != 0)	return PDS_FILE_ERROR;

	*io_count = 0;
	while(1){
		// Read the key and record
		int key_read, fread_status;

		// Break when EOF
		if(feof(fp)){
			return PDS_REC_NOT_FOUND;
		}

		fread(&key_read, sizeof(int), 1, fp);
		*io_count = *io_count + 1;

		// Read record and increment io_count
		fread(rec, repo_handle.linked_rec_size, 1, fp);

		// Check match using matcher function
		if(key_read == key){
			return PDS_SUCCESS;
		}
	}

	// Return failure if record not found
	return PDS_REC_NOT_FOUND;
}

int delete_rec_by_ndx_key(int key)
{
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}

	struct BST_Node * i = bst_search(repo_handle.ndx_root, key);

	// Key not found
	if (i==NULL)	return PDS_DELETE_FAILED;

	struct PDS_NdxInfo *tmp = i->data;
	// Record deleted
	if (tmp->is_deleted == 1)	return PDS_DELETE_FAILED;
	
	// Change in BST
	tmp->is_deleted = 1;

	// Change in data file (Write at the end of the record)
	fseek(repo_handle.pds_data_fp, tmp->offset + sizeof(int) + repo_handle.rec_size, SEEK_SET);
	fwrite(&(tmp->is_deleted), sizeof(int), 1, repo_handle.pds_data_fp);

	return PDS_SUCCESS;
}

int pds_link_rec(int key1, int key2)
{
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}

	// Check if records exist then only link them
	void * parent_rec = malloc(repo_handle.rec_size);
	void * child_rec = malloc(repo_handle.linked_rec_size);
	int io_count;		// Not used but required for callign get_linked_rec_by_key

	int status1 = get_rec_by_ndx_key(key1, parent_rec);
	if (status1 != PDS_SUCCESS)	return -1;

	int status2 = get_linked_rec_by_key(key2, child_rec, &io_count);
	if (status2 != PDS_SUCCESS)	return -1;

	struct PDS_link_info *link_info = malloc(sizeof(struct PDS_link_info));

	if (link_info == NULL)	return PDS_MALLOC_FAILED;

	link_info->parent_key = key1;
	link_info->child_key = key2;

	struct PDS_link_info link_info_test;

	// Check if records are already linked by traversing the link file
	fseek(repo_handle.pds_link_fp, 0, SEEK_SET);
	while (fread(&link_info_test, sizeof(struct PDS_link_info), 1, repo_handle.pds_link_fp) == 1){
		if (link_info_test.parent_key == key1 && link_info_test.child_key == key2){
			return PDS_LINK_EXISTS;
		}
	}

	// Seek to end
	int lp_set = fseek(repo_handle.pds_link_fp, 0, SEEK_END);
	if (lp_set != 0)	return PDS_FILE_ERROR;

	// Write to the file
	fwrite(link_info, sizeof(struct PDS_link_info), 1, repo_handle.pds_link_fp);

	return PDS_SUCCESS;
}

int pds_get_linked_rec(int parent_key, int linked_key_result[], int * result_set_size)
{
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}

	// Check if parent is present in the file
	void * parent_rec = malloc(repo_handle.rec_size);
	int status1 = get_rec_by_ndx_key(parent_key, parent_rec);
	if (status1 != PDS_SUCCESS)	return -1;

	// Seek to the start of the file
	int lp_set = fseek(repo_handle.pds_link_fp, 0, SEEK_SET);
	if (lp_set != 0)	return PDS_FILE_ERROR;

	*result_set_size = 0;

	while(1){
		// Read the key and record
		struct PDS_link_info *link_info = malloc(sizeof(struct PDS_link_info));
		if (feof(repo_handle.pds_link_fp))	return PDS_SUCCESS;
		if (link_info == NULL)	return PDS_MALLOC_FAILED;

		int rec_read = fread(link_info, sizeof(struct PDS_link_info), 1, repo_handle.pds_link_fp);
		if (rec_read != 1)	break;

		if (link_info->parent_key == parent_key){
			linked_key_result[*result_set_size] = link_info->child_key;
			(*result_set_size)++;
			// printf("%d\n", link_info->child_key);		// Testing purposes
		}
	}

	return PDS_SUCCESS;
}

void store_BST_preorder(struct BST_Node *root, FILE *fp){
	// Function to store nodes of BST to file in pre-order (node, left, right)
	if(root == NULL){
		return;
	}

	// Write the ndx entry to file
	struct PDS_NdxInfo ndx_entry;
	ndx_entry.key = root->key;
	ndx_entry.offset = ((struct PDS_NdxInfo *)(root->data))->offset;
	ndx_entry.is_deleted = ((struct PDS_NdxInfo *)(root->data))->is_deleted;
	fwrite(&ndx_entry, sizeof(struct PDS_NdxInfo), 1, fp);

	// Recursively store left and right subtrees
	store_BST_preorder(root->left_child, fp);
	store_BST_preorder(root->right_child, fp);
}

// pds_close
// Open the index file in wb mode (write mode, not append mode)
// Store the number of records
// Unload the ndx_array into the index file by traversing the BST in pre-order mode (overwrite the entire index file)
// Think why should it NOT be in-order?
// Close the index file and data file
int pds_close(){
	// Check if repo is open then do work
	if (repo_handle.repo_status != PDS_REPO_OPEN){
		return PDS_REPO_NOT_OPEN;
	}
	
	// Open the index file in wb mode (write mode, not append mode)
	char ndx_file_name[strlen(repo_handle.pds_name) + 5];
	strcpy(ndx_file_name, repo_handle.pds_name);
	strcat(ndx_file_name, ".ndx");
	FILE * fp2 = fopen(ndx_file_name, "wb");

	if(fp2 == NULL){
		return PDS_FILE_ERROR;
	}
	fseek(fp2, 0, SEEK_SET);

	// Store records
	fwrite(&repo_handle.rec_count, sizeof(int), 1, fp2);

	// Unloading ndx_array into the index file by traversing the BST in pre-order mode
	store_BST_preorder(repo_handle.ndx_root, fp2);
	fclose(fp2);		// Close the index file

	// Close the data file
	int close_status1 = 0, close_status2 = 0, close_status3 = 0;
	close_status1 = fclose(repo_handle.pds_data_fp);

	if (repo_handle.pds_linked_data_fp != NULL)
		close_status2 = fclose(repo_handle.pds_linked_data_fp);
	if (repo_handle.pds_link_fp != NULL)
		close_status3 = fclose(repo_handle.pds_link_fp);

	if (close_status1 != 0 || close_status2 != 0 || close_status3 != 0){
		return PDS_FILE_ERROR;
	}

	// Free memory for BST
	bst_destroy(repo_handle.ndx_root);

	// Change repo status
	repo_handle.repo_status = PDS_REPO_CLOSED;

	return PDS_SUCCESS;
}
