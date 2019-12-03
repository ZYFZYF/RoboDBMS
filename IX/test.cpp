//
// File:        ix_testshell.cc
// Description: Test IX component
// Authors:     Jan Jannink
//              Dallan Quass (quass@cs.stanford.edu)
//
// This test shell contains a number of functions that will be useful in
// testing your IX component code.  In addition, a couple of sample
// tests are provided.  The tests are by no means comprehensive, you are
// expected to devise your own tests to test your code.
//
// 1997:  Tester has been modified to reflect the change in the 1997
// interface.
// 2000:  Tester has been modified to reflect the change in the 2000
// interface.

#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cassert>

#include "IX_Manager.h"
#include "../RM/RM_FileHandle.h"
#include "IX_IndexScan.h"
#include "../RM/RM_Manager.h"
#include "../utils/PrintError.h"
#include "../Attr.h"

using namespace std;

//
// Defines
//
#define FILENAME     "testrel"        // test file name
#define BADFILE      "/abc/def/xyz"   // bad file name
#define STRLEN       39               // length of strings to index
#define FEW_ENTRIES  20
#define MANY_ENTRIES 1000
#define LARGE_ENTRIES 100000
#define NENTRIES     500000             // Size of values array
#define PROG_UNIT    200              // how frequently to give progress
#define STRING_POOL "test.sp"
// reports when adding lots of entries

//
// Values array we will be using for our tests
//
int values[NENTRIES];
Varchar vars[NENTRIES];
int varValues[NENTRIES];


int value1[NENTRIES];
int value2[NENTRIES];

//
// Global component manager variables
//
PF_Manager pfm;
RM_Manager rmm(pfm);
IX_Manager ixm(pfm);

//
// Function declarations
//
RC Test1(void);

RC Test2(void);

RC Test3(void);

RC Test4(void);

RC Test5(void);

RC Test6(void);

RC Test7(void);

RC Test8(void);

RC Test9(void);

RC Test10(void);

RC Test11(void);

RC Test12(void);

RC Test13(void);

RC Test14(void);


void LsFiles(char *fileName);

void ran(int n);

RC InsertIntEntries(IX_IndexHandle &ih, int nEntries);

RC InsertFloatEntries(IX_IndexHandle &ih, int nEntries);

RC InsertStringEntries(IX_IndexHandle &ih, int nEntries);

RC AddRecs(RM_FileHandle &fh, int nRecs);

RC DeleteIntEntries(IX_IndexHandle &ih, int nEntries);

RC DeleteFloatEntries(IX_IndexHandle &ih, int nEntries);

RC DeleteStringEntries(IX_IndexHandle &ih, int nEntries);

RC VerifyIntIndex(IX_IndexHandle &ih, int nStart, int nEntries, int bExists);

RC VerifyStringIndex(IX_IndexHandle &ih, int nStart, int nEntries, int bExists);


RC PrintIndex(IX_IndexHandle &ih);

//
// Array of pointers to the test functions
//
#define NUM_TESTS       14               // number of tests

RC (*tests[])() =                      // RC doesn't work on some compilers
        {
                Test1,
                Test2,
                Test3,
                Test4,
                Test5,
                Test6,
                Test7,
                Test8,
                Test9,
                Test10,
                Test11,
                Test12,
                Test13,
                Test14
        };

//
// main
//
int main(int argc, char *argv[]) {
    RC rc;
    char *progName = argv[0];   // since we will be changing argv
    int testNum;

    // Write out initial starting message
    printf("Starting IX component test.\n\n");

    // Init randomize function
    //srand((unsigned) time(NULL));

    // Delete files from last time (if found)
    // Don't check the return codes, since we expect to get an error
    // if the files are not found.
    rmm.DestroyFile(FILENAME);
    ixm.DestroyIndex(FILENAME, 0);
    ixm.DestroyIndex(FILENAME, 1);
    ixm.DestroyIndex(FILENAME, 2);
    ixm.DestroyIndex(FILENAME, 3);

    // If no argument given, do all tests
    if (argc == 1) {
        for (testNum = 0; testNum < NUM_TESTS; testNum++)
            if ((rc = (tests[testNum])())) {
                // Print the error and exit
                printError(rc);
                return (1);
            }
    } else {
        // Otherwise, perform specific tests
        while (*++argv != NULL) {

            // Make sure it's a number
            if (sscanf(*argv, "%d", &testNum) != 1) {
                cerr << progName << ": " << *argv << " is not a number\n";
                continue;
            }

            // Make sure it's in range
            if (testNum < 1 || testNum > NUM_TESTS) {
                cerr << "Valid test numbers are between 1 and " << NUM_TESTS << "\n";
                continue;
            }

            // Perform the test
            if ((rc = (tests[testNum - 1])())) {
                // Print the error and exit
                printError(rc);
                return (1);
            }
        }
    }

    // Write ending message and exit
    printf("Ending IX component test.\n\n");

    return (0);
}

////////////////////////////////////////////////////////////////////
// The following functions may be useful in tests that you devise //
////////////////////////////////////////////////////////////////////

//
// LsFiles
//
// Desc: list the filename's directory entry
//
void LsFiles(char *fileName) {
    char command[80];

    sprintf(command, "ls -l *%s*", fileName);
    printf("Doing \"%s\"\n", command);
    system(command);
}

//
// Create an array of random nos. between 0 and n-1, without duplicates.
// put the nos. in values[]
//
void ran(int n) {
    int i, r, t, m;

    // Initialize values array
    for (i = 0; i < NENTRIES; i++)
        values[i] = i;

    // Randomize first n entries in values array
    for (i = 0, m = n; i < n - 1; i++) {
        r = (int) (rand() % m--);
        t = values[m];
        values[m] = values[r];
        values[r] = t;
    }

}

//
// InsertIntEntries
//
// Desc: Add a number of integer entries to the index
//
RC InsertIntEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;
    int value;

    printf("             Adding %d int entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        value = values[i] + 1;
        RM_RID rid(value, value * 2);
        //cout << "Insert " << i + 1 << "th key = " << value << " value = (" << value << ',' << value * 2 << ')' << endl;
        if ((rc = ih.InsertEntry((void *) &value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            // cast to long for PC's
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// Desc: Add a number of float entries to the index
//
RC InsertFloatEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;
    float value;

    printf("             Adding %d float entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        value = values[i] + 1;
        RM_RID rid((PageNum) value, (SlotNum) value * 2);
        if ((rc = ih.InsertEntry((void *) &value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// Desc: Add a number of string entries to the index
//
RC InsertStringEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;
    char value[STRLEN];

    printf("             Adding %d string entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        memset(value, ' ', STRLEN);
        sprintf(value, "number %d", values[i] + 1);
        RM_RID rid(values[i] + 1, (values[i] + 1) * 2);
        if ((rc = ih.InsertEntry(value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// InsertVarcharEntries
//
// Desc: Add a number of varchar entries to the index
//
RC InsertVarcharEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;

    printf("             Adding %d varchar entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        RM_RID rid(varValues[i] + 1, (varValues[i] + 1) * 2);
        if ((rc = ih.InsertEntry(&vars[i], rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// Desc: Add a number of union entries to the index
//
RC InsertUnionEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;


    printf("             Adding %d union entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        char temp[STRLEN * 2];
        memset(temp, ' ', STRLEN);
        sprintf(temp, "number %d", value2[i] + 1);

        char value[STRLEN * 2];
        *(AttrType *) value = INT;
        *(int *) (value + ATTR_TYPE_LENGTH) = 4;
        *(int *) (value + ATTR_TYPE_LENGTH + 4) = value1[i];
        *(AttrType *) (value + ATTR_TYPE_LENGTH + 8) = STRING;
        *(int *) (value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH) = STRLEN;
        memcpy(value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4, temp, STRLEN);

        RM_RID rid(value2[i] + 1, (value2[i] + 1) * 2);
        if ((rc = ih.InsertEntry(value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// DeleteIntEntries: delete a number of integer entries from an index
//
RC DeleteIntEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;
    int value;

    printf("        Deleting %d int entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        value = values[i] + 1;
        RM_RID rid(value, value * 2);
        //cout << "Delete " << i + 1 << "th key = " << value << " value = (" << value << ',' << value * 2 << ')' << endl;
        if ((rc = ih.DeleteEntry((void *) &value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%      ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    return OK_RC;
}

//
// DeleteFloatEntries: delete a number of float entries from an index
//
RC DeleteFloatEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;
    float value;

    printf("        Deleting %d float entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        value = values[i] + 1;
        RM_RID rid((PageNum) value, (SlotNum) value * 2);
        if ((rc = ih.DeleteEntry((void *) &value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%      ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    return OK_RC;
}

//
// Desc: Delete a number of string entries from an index
//
RC DeleteStringEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;
    char value[STRLEN + 1];

    printf("             Deleting %d float entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        sprintf(value, "number %d", values[i] + 1);
        RM_RID rid(values[i] + 1, (values[i] + 1) * 2);
        if ((rc = ih.DeleteEntry(value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// DeleteVarcharEntries: delete a number of varchar entries from an index
//
RC DeleteVarcharEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;

    printf("             Deleting %d float entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        RM_RID rid(varValues[i] + 1, (varValues[i] + 1) * 2);
        if ((rc = ih.DeleteEntry(&vars[i], rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// Desc: Delete a number of union entries from an index
//
RC DeleteUnionEntries(IX_IndexHandle &ih, int nEntries) {
    RC rc;
    int i;
    char value[STRLEN + 1];

    printf("             Deleting %d float entries\n", nEntries);
    ran(nEntries);
    for (i = 0; i < nEntries; i++) {
        char temp[STRLEN * 2];
        memset(temp, ' ', STRLEN);
        sprintf(temp, "number %d", value2[i] + 1);

        char value[STRLEN * 2];
        *(AttrType *) value = INT;
        *(int *) (value + ATTR_TYPE_LENGTH) = 4;
        *(int *) (value + ATTR_TYPE_LENGTH + 4) = value1[i];
        *(AttrType *) (value + ATTR_TYPE_LENGTH + 8) = STRING;
        *(int *) (value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH) = STRLEN;
        memcpy(value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4, temp, STRLEN);

        RM_RID rid(value2[i] + 1, (value2[i] + 1) * 2);
        if ((rc = ih.DeleteEntry(value, rid)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("\r\t%d%%    ", (int) ((i + 1) * 100L / nEntries));
            fflush(stdout);
        }
    }
    printf("\r\t%d%%      \n", (int) (i * 100L / nEntries));

    // Return ok
    return OK_RC;
}

//
// VerifyIntIndex
//   - nStart is the starting point in the values array to check
//   - nEntries is the number of entries in the values array to check
//   - If bExists == 1, verify that an index has entries as added
//     by InsertIntEntries.
//     If bExists == 0, verify that entries do NOT exist (you can
//     use this to test deleting entries).
//
RC VerifyIntIndex(IX_IndexHandle &ih, int nStart, int nEntries, int bExists) {
    RC rc;
    int i;
    RM_RID rid;
    IX_IndexScan scan;
    PageNum pageNum;
    SlotNum slotNum;

    // Assume values still contains the array of values inserted/deleted

    printf("Verifying string index contents\n");

    for (i = nStart; i < nStart + nEntries; i++) {
        int value = values[i] + 1;

        if ((rc = scan.OpenScan(ih, EQ_OP, &value))) {
            printf("Verify error: opening scan\n");
            return (rc);
        }

        rc = scan.GetNextEntry(rid);
        if (!bExists && rc == 0) {
            printf("Verify error: found non-existent %dth entry %d\n", i - nStart + 1, value);
            return IX_FIND_NON_EXIST;
        } else if (bExists && rc == IX_EOF) {
            printf("Verify error: %dth entry %d not found\n", i - nStart + 1, value);
            return IX_NOT_FIND;
        } else if (rc != 0 && rc != IX_EOF)
            return (rc);
        else {
            //printf("Verify %dth entry %d\n", i - nStart + 1, value);
        }

        if (bExists && rc == 0) {
            // Did we get the right entry?
            if ((rc = rid.GetPageNum(pageNum)) ||
                (rc = rid.GetSlotNum(slotNum)))
                return (rc);

            if (pageNum != value || slotNum != (value * 2)) {
                printf("Verify error: incorrect rid (%d,%d) found for entry %d\n",
                       pageNum, slotNum, value);
                return (IX_EOF);  // What should be returned here?
            }

            // Is there another entry?
            rc = scan.GetNextEntry(rid);
            if (rc == 0) {
                printf("Verify error: found two entries with same value %d\n", value);
                return (IX_EOF);  // What should be returned here?
            } else if (rc != IX_EOF)
                return (rc);
        }

        if ((rc = scan.CloseScan())) {
            printf("Verify error: closing scan\n");
            return (rc);
        }
    }

    return OK_RC;
}

RC VerifyStringIndex(IX_IndexHandle &ih, int nStart, int nEntries, int bExists) {
    RC rc;
    int i;
    RM_RID rid;
    IX_IndexScan scan;
    PageNum pageNum;
    SlotNum slotNum;

    // Assume values still contains the array of values inserted/deleted

    printf("Verifying index contents\n");
    char value[STRLEN + 1];

    for (i = nStart; i < nStart + nEntries; i++) {
        sprintf(value, "number %d", values[i] + 1);

        if ((rc = scan.OpenScan(ih, EQ_OP, value))) {
            printf("Verify error: opening scan\n");
            return (rc);
        }

        rc = scan.GetNextEntry(rid);
        if (!bExists && rc == 0) {
            printf("Verify error: found non-existent %dth entry %s\n", i - nStart + 1, value);
            return IX_FIND_NON_EXIST;  // What should be returned here?
        } else if (bExists && rc == IX_EOF) {
            printf("Verify error: %dth entry %s not found\n", i - nStart + 1, value);
            return IX_NOT_FIND;  // What should be returned here?
        } else if (rc != 0 && rc != IX_EOF)
            return (rc);
        else {
            //printf("Verify %dth entry %d\n", i - nStart + 1, value);
        }

        if (bExists && rc == 0) {
            // Did we get the right entry?
            if ((rc = rid.GetPageNum(pageNum)) ||
                (rc = rid.GetSlotNum(slotNum)))
                return (rc);

            if (pageNum != (values[i] + 1) || slotNum != ((values[i] + 1) * 2)) {
                printf("Verify error: incorrect rid (%d,%d) found for entry %s\n",
                       pageNum, slotNum, value);
                return (IX_EOF);  // What should be returned here?
            }

            // Is there another entry?
            rc = scan.GetNextEntry(rid);
            if (rc == 0) {
                printf("Verify error: found two entries with same value %d\n", value);
                return (IX_EOF);  // What should be returned here?
            } else if (rc != IX_EOF)
                return (rc);
        }

        if ((rc = scan.CloseScan())) {
            printf("Verify error: closing scan\n");
            return (rc);
        }
    }

    return OK_RC;
}

RC VerifyVarcharIndex(IX_IndexHandle &ih, int nStart, int nEntries, int bExists) {
    RC rc;
    int i;
    RM_RID rid;
    IX_IndexScan scan;
    PageNum pageNum;
    SlotNum slotNum;

    // Assume values still contains the array of values inserted/deleted

    printf("Verifying index contents\n");

    for (i = nStart; i < nStart + nEntries; i++) {

        if ((rc = scan.OpenScan(ih, EQ_OP, &vars[i]))) {
            printf("Verify error: opening scan\n");
            return (rc);
        }

        rc = scan.GetNextEntry(rid);
        if (!bExists && rc == 0) {
            printf("Verify error: found non-existent %dth entry %s\n", i - nStart + 1, vars[i]);
            return IX_FIND_NON_EXIST;  // What should be returned here?
        } else if (bExists && rc == IX_EOF) {
            printf("Verify error: %dth entry %s not found\n", i - nStart + 1, vars[i]);
            return IX_NOT_FIND;  // What should be returned here?
        } else if (rc != 0 && rc != IX_EOF)
            return (rc);
        else {
            //printf("Verify %dth entry %d\n", i - nStart + 1, value);
        }

        if (bExists && rc == 0) {
            // Did we get the right entry?
            if ((rc = rid.GetPageNum(pageNum)) ||
                (rc = rid.GetSlotNum(slotNum)))
                return (rc);
            if (pageNum != (varValues[i] + 1) || slotNum != ((varValues[i] + 1) * 2)) {
                printf("Verify error: incorrect rid (%d,%d) found for entry %s\n",
                       pageNum, slotNum, vars[i]);
                return (IX_EOF);  // What should be returned here?
            }
            char value[100];
            vars[i].getData(value);
            //printf("%d %s\n", varValues[i], value);

            // Is there another entry?
            rc = scan.GetNextEntry(rid);
            if (rc == 0) {
                printf("Verify error: found two entries with same value %d\n", vars[i]);
                return (IX_EOF);  // What should be returned here?
            } else if (rc != IX_EOF)
                return (rc);
        }

        if ((rc = scan.CloseScan())) {
            printf("Verify error: closing scan\n");
            return (rc);
        }
    }

    return OK_RC;
}

RC VerifyUnionIndex(IX_IndexHandle &ih, int nStart, int nEntries, int bExists) {
    RC rc;
    int i;
    RM_RID rid;
    IX_IndexScan scan;
    PageNum pageNum;
    SlotNum slotNum;

    // Assume values still contains the array of values inserted/deleted

    printf("Verifying index contents\n");

    for (i = nStart; i < nStart + nEntries; i++) {
        char temp[STRLEN * 2];
        memset(temp, ' ', STRLEN);
        sprintf(temp, "number %d", value2[i] + 1);
        char value[STRLEN * 2];
        *(AttrType *) value = INT;
        *(int *) (value + ATTR_TYPE_LENGTH) = 4;
        *(int *) (value + ATTR_TYPE_LENGTH + 4) = value1[i];
        *(AttrType *) (value + ATTR_TYPE_LENGTH + 8) = STRING;
        *(int *) (value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH) = STRLEN;
        memcpy(value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4, temp, STRLEN);

        if ((rc = scan.OpenScan(ih, EQ_OP, value))) {
            printf("Verify error: opening scan\n");
            return (rc);
        }

        rc = scan.GetNextEntry(rid);
        if (!bExists && rc == 0) {
            printf("Verify error: found non-existent %dth entry %s\n", i - nStart + 1, value);
            return IX_FIND_NON_EXIST;  // What should be returned here?
        } else if (bExists && rc == IX_EOF) {
            printf("Verify error: %dth entry %s not found\n", i - nStart + 1, value);
            return IX_NOT_FIND;  // What should be returned here?
        } else if (rc != 0 && rc != IX_EOF)
            return (rc);
        else {
            //printf("Verify %dth entry %d\n", i - nStart + 1, value);
        }

        if (bExists && rc == 0) {
            // Did we get the right entry?
            if ((rc = rid.GetPageNum(pageNum)) ||
                (rc = rid.GetSlotNum(slotNum)))
                return (rc);

            if (pageNum != (value2[i] + 1) || slotNum != ((value2[i] + 1) * 2)) {
                printf("Verify error: incorrect rid (%d,%d) found for entry %s\n",
                       pageNum, slotNum, value);
                return (IX_EOF);  // What should be returned here?
            }

            // Is there another entry?
            rc = scan.GetNextEntry(rid);
            if (rc == 0) {
                printf("Verify error: found two entries with same value %d\n", value);
                return (IX_EOF);  // What should be returned here?
            } else if (rc != IX_EOF)
                return (rc);
        }

        if ((rc = scan.CloseScan())) {
            printf("Verify error: closing scan\n");
            return (rc);
        }
    }

    return OK_RC;
}




/////////////////////////////////////////////////////////////////////
// Sample test functions follow.                                   //
/////////////////////////////////////////////////////////////////////

//
// Test1 tests simple creation, opening, closing, and deletion of indices
//
RC Test1(void) {
    RC rc;
    int index = 0;
    IX_IndexHandle ih;

    printf("Test 1: create, open, close, delete an index... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 1\n\n");
    return OK_RC;
}

//
// Test2 tests inserting a few integer entries into the index.
//
RC Test2(void) {
    RC rc;
    IX_IndexHandle ih;
    int index = 0;

    printf("Test2: Insert a few integer entries into an index... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, FEW_ENTRIES)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||

        // ensure inserted entries are all there
        (rc = VerifyIntIndex(ih, 0, FEW_ENTRIES, TRUE)) ||

        // ensure an entry not inserted is not there
        (rc = VerifyIntIndex(ih, FEW_ENTRIES, 1, FALSE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 2\n\n");
    return OK_RC;
}

//
// Test5 tests inserting number of integer entries into the index.
//
RC Test5(void) {
    RC rc;
    IX_IndexHandle ih;
    int index = 0;

    printf("Test5: Insert number of integer entries into an index... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, MANY_ENTRIES)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||

        // ensure inserted entries are all there
        (rc = VerifyIntIndex(ih, 0, MANY_ENTRIES, TRUE)) ||

        // ensure an entry not inserted is not there
        (rc = VerifyIntIndex(ih, MANY_ENTRIES, 1, FALSE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 5\n\n");
    return OK_RC;
}

//
// Test6 tests inserting large number of integer entries into the index.
//
RC Test6(void) {
    RC rc;
    IX_IndexHandle ih;
    int index = 0;

    printf("Test6: Insert large number of integer entries into an index... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, LARGE_ENTRIES)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||

        // ensure inserted entries are all there
        (rc = VerifyIntIndex(ih, 0, LARGE_ENTRIES, TRUE)) ||

        // ensure an entry not inserted is not there
        (rc = VerifyIntIndex(ih, LARGE_ENTRIES, 1, FALSE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 6\n\n");
    return OK_RC;
}

//
// Test3 tests deleting a few integer entries from an index
//
RC Test3(void) {
    RC rc;
    int index = 0;
    int nDelete = FEW_ENTRIES * 8 / 10;
    IX_IndexHandle ih;

    printf("Test3: Delete a few integer entries from an index... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, FEW_ENTRIES)) ||
        (rc = DeleteIntEntries(ih, nDelete)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        // ensure deleted entries are gone
        (rc = VerifyIntIndex(ih, 0, nDelete, FALSE)) ||
        // ensure non-deleted entries still exist
        (rc = VerifyIntIndex(ih, nDelete, FEW_ENTRIES - nDelete, TRUE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 3\n\n");
    return OK_RC;
}

//
// Test 4 tests a few inequality scans on Btree indices
//
RC Test4(void) {
    RC rc;
    IX_IndexHandle ih;

    int index = 0;
    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, FEW_ENTRIES)))
        return (rc);
    int i;
    int valueList[] = {0, 1, -1, FEW_ENTRIES / 4, FEW_ENTRIES / 2, FEW_ENTRIES - 1, FEW_ENTRIES, FEW_ENTRIES + 1};
    printf("Test4: Inequality scans... \n");
    for (int value : valueList) {
        RM_RID rid;



        // Scan <
        IX_IndexScan scanlt;
        if ((rc = scanlt.OpenScan(ih, LT_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scanlt.GetNextEntry(rid))) {
            i++;
        }

        if (rc != IX_EOF)
            return (rc);

        printf("Found %d entries in < %d\n", i, value);

        // Scan <=
        IX_IndexScan scanle;
        if ((rc = scanle.OpenScan(ih, LE_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scanle.GetNextEntry(rid))) {
            i++;
        }
        if (rc != IX_EOF)
            return (rc);

        printf("Found %d entries in <= %d\n", i, value);

        // Scan >
        IX_IndexScan scangt;
        if ((rc = scangt.OpenScan(ih, GT_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scangt.GetNextEntry(rid))) {
            i++;
        }
        if (rc != IX_EOF)
            return (rc);

        printf("Found %d entries in > %d\n", i, value);

        // Scan >=
        IX_IndexScan scange;
        if ((rc = scange.OpenScan(ih, GE_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scange.GetNextEntry(rid))) {
            i++;
        }
        if (rc != IX_EOF)
            return (rc);

        printf("Found %d entries in >= %d\n", i, value);

        // Scan !=
        IX_IndexScan scanne;
        if ((rc = scanne.OpenScan(ih, NE_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scanne.GetNextEntry(rid))) {
            i++;
        }
        if (rc != IX_EOF)
            return (rc);

        printf("Found %d entries in != %d\n", i, value);
    }


    if ((rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 4\n\n");
    return OK_RC;
}

RC Test7(void) {
    RC rc;
    int index = 1;
    IX_IndexHandle ih;
    printf("Test7: Some special insert and delete... \n");
    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, FEW_ENTRIES)))
        return (rc);
    int key = values[0] + 1;
    RM_RID rmRid(key, key * 2);
    if (ih.InsertEntry(&key, rmRid) == 0) {
        return IX_INSERT_TWICE;
    }
    TRY(DeleteIntEntries(ih, FEW_ENTRIES));
    if (ih.DeleteEntry(&key, rmRid) == 0) {
        return IX_DELETE_TWICE;
    };
    key++;
    if (ih.DeleteEntry(&key, rmRid) == 0) {
        return IX_DELETE_NON_EXIST;
    }
    if ((rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 7\n\n");
    return OK_RC;
}

//
// Test8 tests delete many integer entries into the index.
//
RC Test8(void) {
    RC rc;
    int index = 0;
    int nDelete = MANY_ENTRIES * 8 / 10;
    IX_IndexHandle ih;

    printf("Test8: Delete many integer entries from an index... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, MANY_ENTRIES)) ||
        (rc = DeleteIntEntries(ih, nDelete)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        // ensure deleted entries are gone
        (rc = VerifyIntIndex(ih, 0, nDelete, FALSE)) ||
        // ensure non-deleted entries still exist
        (rc = VerifyIntIndex(ih, nDelete, MANY_ENTRIES - nDelete, TRUE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 8\n\n");
    return OK_RC;
}

//
// Test9 tests delete large number of integer entries into the index.
//
RC Test9(void) {
    RC rc;
    int index = 0;
    int nDelete = LARGE_ENTRIES * 8 / 10;
    IX_IndexHandle ih;

    printf("Test9: Delete large number of integer entries from an index... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, LARGE_ENTRIES)) ||
        (rc = DeleteIntEntries(ih, nDelete)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        // ensure deleted entries are gone
        (rc = VerifyIntIndex(ih, 0, nDelete, FALSE)) ||
        // ensure non-deleted entries still exist
        (rc = VerifyIntIndex(ih, nDelete, LARGE_ENTRIES - nDelete, TRUE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 9\n\n");
    return OK_RC;
}

//
// Test10 tests delete B+ tree to empty
//
RC Test10(void) {
    RC rc;
    int index = 0;
    int nDelete = LARGE_ENTRIES;
    IX_IndexHandle ih;

    printf("Test10: delete B+ tree to empty... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, LARGE_ENTRIES)) ||
        (rc = DeleteIntEntries(ih, nDelete)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        // ensure deleted entries are gone
        (rc = VerifyIntIndex(ih, 0, nDelete, FALSE)) ||
        // ensure non-deleted entries still exist
        (rc = VerifyIntIndex(ih, nDelete, LARGE_ENTRIES - nDelete, TRUE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 10fD\n\n");
    return OK_RC;
}
//TODO  其他类型的测试

//
// Test 11 tests a few inequality scans on large Btree indices
//
RC Test11(void) {
    RC rc;
    IX_IndexHandle ih;
    int total_entry = LARGE_ENTRIES;
    int index = 0;
    if ((rc = ixm.CreateIndex(FILENAME, index, INT, sizeof(int))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertIntEntries(ih, total_entry)))
        return (rc);
    int i;
    int valueList[] = {0, 1, -1, total_entry / 4, total_entry / 2, total_entry - 1, total_entry, total_entry + 1};
    printf("Test11: a few inequality scans on large Btree indices... \n");
    for (int value : valueList) {
        RM_RID rid;
        // Scan <
        IX_IndexScan scanlt;
        if ((rc = scanlt.OpenScan(ih, LT_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scanlt.GetNextEntry(rid))) {
            int pageNum, slotNum;
            rid.GetPageNumAndSlotNum(pageNum, slotNum);
            //printf("scan %d %d\n", pageNum, slotNum);
            i++;
        }

        if (rc != IX_EOF)
            return (rc);
        printf("Found %d entries in < %d\n", i, value);
        assert(i == (value <= 0 ? 0 : value > total_entry ? total_entry : value - 1));

        // Scan <=
        IX_IndexScan scanle;
        if ((rc = scanle.OpenScan(ih, LE_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scanle.GetNextEntry(rid))) {
            i++;
        }
        if (rc != IX_EOF)
            return (rc);
        printf("Found %d entries in <= %d\n", i, value);
        assert(i == (value <= 0 ? 0 : value > total_entry ? total_entry : value));

        // Scan >
        IX_IndexScan scangt;
        if ((rc = scangt.OpenScan(ih, GT_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scangt.GetNextEntry(rid))) {
            i++;
            int pageNum, slotNum;
            rid.GetPageNumAndSlotNum(pageNum, slotNum);
            //printf("scan %dth %d %d\n", i, pageNum, slotNum);
        }
        if (rc != IX_EOF)
            return (rc);
        printf("Found %d entries in > %d\n", i, value);
        assert(i == (value <= 0 ? total_entry : value > total_entry ? 0 : total_entry - value));

        // Scan >=
        IX_IndexScan scange;
        if ((rc = scange.OpenScan(ih, GE_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scange.GetNextEntry(rid))) {
            i++;
        }
        if (rc != IX_EOF)
            return (rc);
        printf("Found %d entries in >= %d\n", i, value);
        assert(i == (value <= 0 ? total_entry : value > total_entry ? 0 : total_entry - value + 1));

        // Scan !=
        IX_IndexScan scanne;
        if ((rc = scanne.OpenScan(ih, NE_OP, &value))) {
            printf("Scan error: opening scan\n");
            return (rc);
        }

        i = 0;
        while (!(rc = scanne.GetNextEntry(rid))) {
            i++;
        }
        if (rc != IX_EOF)
            return (rc);
        printf("Found %d entries in != %d\n", i, value);
        assert(i == (value > 0 && value <= total_entry) ? total_entry - 1 : total_entry);
    }


    if ((rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 11\n\n");
    return OK_RC;
}

//
// Test12 tests string.
//
RC Test12(void) {
    RC rc;
    int index = 0;
    int nDelete = LARGE_ENTRIES * 8 / 10;
    IX_IndexHandle ih;

    printf("Test12: test string index insert and delete... \n");

    if ((rc = ixm.CreateIndex(FILENAME, index, STRING, STRLEN)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertStringEntries(ih, LARGE_ENTRIES)) ||
        (rc = VerifyStringIndex(ih, 0, LARGE_ENTRIES, TRUE)) ||
        (rc = DeleteStringEntries(ih, nDelete)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        // ensure deleted entries are gone
        (rc = VerifyStringIndex(ih, 0, nDelete, FALSE)) ||
        // ensure non-deleted entries still exist
        (rc = VerifyStringIndex(ih, nDelete, LARGE_ENTRIES - nDelete, TRUE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 12\n\n");
    return OK_RC;
}

//
// Test13 test index for Varchar
//
RC Test13(void) {
    RC rc;
    int index = 0;
    int nInsert = MANY_ENTRIES;
    int nDelete = nInsert * 8 / 10;
    IX_IndexHandle ih;

    printf("Test13: test varchar index insert and delete... \n");

    ran(nInsert);
    SP_Handle spHandle;
    if (access(STRING_POOL, F_OK) == 0) {
        remove(STRING_POOL);
    }
    SP_Manager::CreateStringPool(STRING_POOL);
    SP_Manager::OpenStringPool(STRING_POOL, spHandle);
    char value[STRLEN + 1];
    for (int i = 0; i < nInsert; i++) {
        varValues[i] = values[i];
        sprintf(value, "number %d", values[i] + 1);
        vars[i].length = strlen(value);
        strcpy(vars[i].spName, STRING_POOL);
        spHandle.InsertString(value, vars[i].length, vars[i].offset);
    }
    SP_Manager::CloseStringPool(spHandle);

    if ((rc = ixm.CreateIndex(FILENAME, index, VARCHAR, sizeof(Varchar))) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertVarcharEntries(ih, nInsert)) ||
        (rc = VerifyVarcharIndex(ih, 0, nInsert, TRUE)) ||
        (rc = DeleteVarcharEntries(ih, nDelete)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        // ensure deleted entries are gone
        (rc = VerifyVarcharIndex(ih, 0, nDelete, FALSE)) ||
        // ensure non-deleted entries still exist
        (rc = VerifyVarcharIndex(ih, nDelete, nInsert - nDelete, TRUE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    TRY(ixm.OpenIndex(FILENAME, index, ih));
    cout << "scan < " << varValues[0] << endl;
    IX_IndexScan scanlt;
    if ((rc = scanlt.OpenScan(ih, LT_OP, &vars[0]))) {
        printf("Scan error: opening scan\n");
        return (rc);
    }
    RM_RID rid;
    int i = 0;
    while (!(rc = scanlt.GetNextEntry(rid))) {
        int pageNum, slotNum;
        rid.GetPageNumAndSlotNum(pageNum, slotNum);
        printf("scan found %d\n", pageNum);
        i++;
    }

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);
    SP_Manager::DestroyStringPool(STRING_POOL);

    printf("Passed Test 13\n\n");
    return OK_RC;
}

//
// Test14 test union index
//
RC Test14(void) {
    cout << ATTR_TYPE_LENGTH << endl;
    RC rc;
    int index = 0;
    int nInsert = MANY_ENTRIES;
    int nDelete = nInsert * 8 / 10;
    IX_IndexHandle ih;

    printf("Test14: test union index... \n");

    for (int i = 0; i < nInsert; i++)value1[i] = rand() % 100;
    for (int i = 0; i < nInsert; i++)value2[i] = i;


    if ((rc = ixm.CreateIndex(FILENAME, index, ATTRARRAY, ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4 + STRLEN)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        (rc = InsertUnionEntries(ih, nInsert)) ||
        (rc = VerifyUnionIndex(ih, 0, nInsert, TRUE)) ||
        (rc = DeleteUnionEntries(ih, nDelete)) ||
        (rc = ixm.CloseIndex(ih)) ||
        (rc = ixm.OpenIndex(FILENAME, index, ih)) ||
        // ensure deleted entries are gone
        (rc = VerifyUnionIndex(ih, 0, nDelete, FALSE)) ||
        // ensure non-deleted entries still exist
        (rc = VerifyUnionIndex(ih, nDelete, nInsert - nDelete, TRUE)) ||
        (rc = ixm.CloseIndex(ih)))
        return (rc);

    TRY(ixm.OpenIndex(FILENAME, index, ih));

    char temp[STRLEN * 2];
    memset(temp, ' ', STRLEN);
    sprintf(temp, "number %d", value2[88] + 1);

    char value[STRLEN * 2];
    *(AttrType *) value = INT;
    *(int *) (value + ATTR_TYPE_LENGTH) = 4;
    *(int *) (value + ATTR_TYPE_LENGTH + 4) = value1[88];
    *(AttrType *) (value + ATTR_TYPE_LENGTH + 8) = STRING;
    *(int *) (value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH) = STRLEN;
    memcpy(value + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4, temp, STRLEN);

    cout << "scan < " << value2[88] << endl;
    IX_IndexScan scanlt;
    if ((rc = scanlt.OpenScan(ih, LT_OP, value))) {
        printf("Scan error: opening scan\n");
        return (rc);
    }
    RM_RID rid;
    int i = 0;
    while (!(rc = scanlt.GetNextEntry(rid))) {
        int pageNum, slotNum;
        rid.GetPageNumAndSlotNum(pageNum, slotNum);
        printf("scan found value1 = %d and value2 = %s and pageNum = %d and slotNum = %d\n",
               *(int *) ((char *) scanlt.getCurrentKey() + ATTR_TYPE_LENGTH + 4),
               (char *) scanlt.getCurrentKey() + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4, pageNum, slotNum);
        i++;
    }

    LsFiles(FILENAME);

    if ((rc = ixm.DestroyIndex(FILENAME, index)))
        return (rc);

    printf("Passed Test 14\n\n");
    return OK_RC;
}
