//
// Created by 赵鋆峰 on 2019/10/27.
//
//
// File:        rm_testshell.cc
// Description: Test RM component
// Authors:     Jan Jannink
//              Dallan Quass (quass@cs.stanford.edu)
//              Jason McHugh (mchughj@cs.stanford.edu)
//
// This test shell contains a number of functions that will be useful
// in testing your RM component code.  In addition, a couple of sample
// tests are provided.  The tests are by no means comprehensive, however,
// and you are expected to devise your own tests to test your code.
//
// 1997:  Tester has been modified to reflect the change in the 1997
// interface.  For example, FileHandle no longer supports a Scan over the
// relation.  All scans are done via a FileScan.
//

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include "RM_Manager.h"
#include "RM_FileScan.h"

using namespace std;

//
// Defines
//
#define FILENAME   "testrel"         // test file name
#define STRLEN      29               // length of string in testrec
#define PROG_UNIT   500               // how frequently to give progress
//   reports when adding lots of recs
#define FEW_RECS   20                // number of records added in
#define LOTS_OF_RECS 12345

//
// Computes the offset of a field in a record (should be in <stddef.h>)
//
#ifndef offsetof
#       define offsetof(type, field)   ((size_t)&(((type *)0) -> field))
#endif

//
// Structure of the records we will be using for the tests
//
struct TestRec {
    char str[STRLEN];
    int num;
    float r;
};

//
// Global PF_Manager and RM_Manager variables
//
PF_Manager pfm;
RM_Manager rmm(pfm);

//
// Function declarations
//
RC Test1(void);

RC Test2(void);

RC Test3(void);

RC Test4(void);

RC Test5(void);

RC Test6(void);

void PrintError(RC rc);

void LsFile(char *fileName);

void PrintRecord(TestRec &recBuf);

RC AddRecs(RM_FileHandle &fh, int numRecs);

RC VerifyFile(RM_FileHandle &fh, int numRecs);

RC PrintFile(RM_FileHandle &fh);

RC CreateFile(char *fileName, int recordSize);

RC DestroyFile(char *fileName);

RC OpenFile(char *fileName, RM_FileHandle &fh);

RC CloseFile(char *fileName, RM_FileHandle &fh);

RC InsertRec(RM_FileHandle &fh, char *record, RM_RID &RM_RID);

RC UpdateRec(RM_FileHandle &fh, RM_Record &rec);

RC DeleteRec(RM_FileHandle &fh, RM_RID &RM_RID);

RC GetNextRecScan(RM_FileScan &fs, RM_Record &rec);

//
// Array of pointers to the test functions
//
RC (*tests[])() =                      // RC doesn't work on some compilers
        {
                Test1,
                Test2,
                Test3,
                Test4,
                Test5,
                Test6,
        };

#define NUM_TESTS       ((int)((sizeof(tests)) / sizeof(tests[0])))    // number of tests

//
// main
//
int main(int argc, char *argv[]) {
    RC rc;
    char *progName = argv[0];   // since we will be changing argv
    int testNum;

    // Write out initial starting message
    cerr.flush();
    cout.flush();
    cout << "Starting RM component test.\n";
    cout.flush();

    // Delete files from last time
    unlink(FILENAME);

    // If no argument given, do all tests
    if (argc == 1) {
        for (testNum = 0; testNum < NUM_TESTS; testNum++)
            if ((rc = (tests[testNum])())) {

                // Print the error and exit
                PrintError(rc);
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
                PrintError(rc);
                return (1);
            }
        }
    }

    // Write ending message and exit
    cout << "Ending RM component test.\n\n";

    return (0);
}

//
// PrintError
//
// Desc: Print an error message by calling the proper component-specific
//       print-error function
//

////////////////////////////////////////////////////////////////////
// The following functions may be useful in tests that you devise //
////////////////////////////////////////////////////////////////////

//
// LsFile
//
// Desc: list the filename's directory entry
//
void LsFile(char *fileName) {
    char command[80];

    sprintf(command, "ls -l %s", fileName);
    printf("doing \"%s\"\n", command);
    system(command);
}

//
// PrintRecord
//
// Desc: Print the TestRec record components
//
void PrintRecord(TestRec &recBuf) {
    printf("[%s, %d, %f]\n", recBuf.str, recBuf.num, recBuf.r);
}

//
// AddRecs
//
// Desc: Add a number of records to the file
//
RC AddRecs(RM_FileHandle &fh, int numRecs) {
    RC rc;
    int i;
    TestRec recBuf;
    RM_RID RM_RID;
    PageNum pageNum;
    SlotNum slotNum;

    // We set all of the TestRec to be 0 initially.  This heads off
    // warnings that Purify will give regarding UMR since sizeof(TestRec)
    // is 40, whereas actual size is 37.
    memset((void *) &recBuf, 0, sizeof(recBuf));

    printf("\nadding %d records\n", numRecs);
    for (i = 0; i < numRecs; i++) {
        memset(recBuf.str, ' ', STRLEN);
        sprintf(recBuf.str, "a%d", i);
        recBuf.num = i;
        recBuf.r = (float) i;
        if ((rc = InsertRec(fh, (char *) &recBuf, RM_RID)) ||
            (rc = RM_RID.GetPageNum(pageNum)) ||
            (rc = RM_RID.GetSlotNum(slotNum)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0) {
            printf("%d  ", i + 1);
            fflush(stdout);
        }
    }
    if (i % PROG_UNIT != 0)
        printf("%d\n", i);
    else
        putchar('\n');

    printf("Page/Slot: %d %d\n", pageNum, slotNum);

    // Return ok
    return OK_RC;
}

//
// VerifyFile
//
// Desc: verify that a file has records as added by AddRecs
//
RC VerifyFile(RM_FileHandle &fh, int numRecs) {
    RC rc;
    int n;
    TestRec *pRecBuf;
    RM_RID RM_RID;
    char stringBuf[STRLEN];
    char *found;
    RM_Record rec;

    found = new char[numRecs];
    memset(found, 0, numRecs);

    printf("\nverifying file contents\n");

    RM_FileScan fs;
    if ((rc = fs.OpenScan(fh, INT, sizeof(int), offsetof(TestRec, num),
                          NO_OP, NULL, NO_HINT)))
        return (rc);

    // For each record in the file
    for (rc = GetNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fs, rec), n++) {

        // Make sure the record is correct
        if ((rc = rec.GetData((char *&) pRecBuf)) ||
            (rc = rec.GetRid(RM_RID)))
            goto err;

        memset(stringBuf, ' ', STRLEN);
        sprintf(stringBuf, "a%d", pRecBuf->num);

        if (pRecBuf->num < 0 || pRecBuf->num >= numRecs ||
            strcmp(pRecBuf->str, stringBuf) ||
            pRecBuf->r != (float) pRecBuf->num) {
            printf("VerifyFile: invalid record = [%s, %d, %f]\n",
                   pRecBuf->str, pRecBuf->num, pRecBuf->r);
            exit(1);
        }

        if (found[pRecBuf->num]) {
            printf("VerifyFile: duplicate record = [%s, %d, %f]\n",
                   pRecBuf->str, pRecBuf->num, pRecBuf->r);
            exit(1);
        }

        found[pRecBuf->num] = 1;
    }

    if (rc != RM_EOF)
        goto err;

    if ((rc = fs.CloseScan()))
        return (rc);

    // make sure we had the right number of records in the file
    if (n != numRecs) {
        printf("%d records in file (supposed to be %d)\n",
               n, numRecs);
        exit(1);
    }

    // Return ok
    rc = OK_RC;

    err:
    fs.CloseScan();
    delete[] found;
    return (rc);
}

//
// PrintFile
//
// Desc: Print the contents of the file
//
RC PrintFile(RM_FileScan &fs) {
    RC rc;
    int n;
    TestRec *pRecBuf;
    RM_RID RM_RID;
    RM_Record rec;

    printf("\nprinting file contents\n");

    // for each record in the file
    for (rc = GetNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fs, rec), n++) {

        // Get the record data and record id
        if ((rc = rec.GetData((char *&) pRecBuf)) ||
            (rc = rec.GetRid(RM_RID)))
            return (rc);

        // Print the record contents
        PrintRecord(*pRecBuf);
    }

    if (rc != RM_EOF)
        return (rc);

    printf("%d records found\n", n);

    // Return ok
    return OK_RC;
}

////////////////////////////////////////////////////////////////////////
// The following functions are wrappers for some of the RM component  //
// methods.  They give you an opportunity to add debugging statements //
// and/or set breakpoints when testing these methods.                 //
////////////////////////////////////////////////////////////////////////

//
// CreateFile
//
// Desc: call RM_Manager::CreateFile
//
RC CreateFile(char *fileName, int recordSize) {
    printf("\ncreating %s\n", fileName);
    return (rmm.CreateFile(fileName, recordSize));
}

//
// DestroyFile
//
// Desc: call RM_Manager::DestroyFile
//
RC DestroyFile(char *fileName) {
    printf("\ndestroying %s\n", fileName);
    return (rmm.DestroyFile(fileName));
}

//
// OpenFile
//
// Desc: call RM_Manager::OpenFile
//
RC OpenFile(char *fileName, RM_FileHandle &fh) {
    printf("\nopening %s\n", fileName);
    return (rmm.OpenFile(fileName, fh));
}

//
// CloseFile
//
// Desc: call RM_Manager::CloseFile
//
RC CloseFile(char *fileName, RM_FileHandle &fh) {
    if (fileName != NULL)
        printf("\nClosing %s\n", fileName);
    return (rmm.CloseFile(fh));
}

//
// InsertRec
//
// Desc: call RM_FileHandle::InsertRec
//
RC InsertRec(RM_FileHandle &fh, char *record, RM_RID &RM_RID) {
    return (fh.InsertRec(record, RM_RID));
}

//
// DeleteRec
//
// Desc: call RM_FileHandle::DeleteRec
//
RC DeleteRec(RM_FileHandle &fh, RM_RID &RM_RID) {
    return (fh.DeleteRec(RM_RID));
}

//
// UpdateRec
//
// Desc: call RM_FileHandle::UpdateRec
//
RC UpdateRec(RM_FileHandle &fh, RM_Record &rec) {
    return (fh.UpdateRec(rec));
}

//
// GetNextRecScan
//
// Desc: call RM_FileScan::GetNextRec
//
RC GetNextRecScan(RM_FileScan &fs, RM_Record &rec) {
    return (fs.GetNextRec(rec));
}

/////////////////////////////////////////////////////////////////////
// Sample test functions follow.                                   //
/////////////////////////////////////////////////////////////////////

//
// Test1 tests simple creation, opening, closing, and deletion of files
//
RC Test1(void) {
    RC rc;
    RM_FileHandle fh;

    printf("test1 starting ****************\n");

    if ((rc = CreateFile((char *) FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char *) FILENAME, fh)) ||
        (rc = CloseFile((char *) FILENAME, fh)))
        return (rc);

    LsFile((char *) FILENAME);

    if ((rc = DestroyFile((char *) FILENAME)))
        return (rc);

    printf("\ntest1 done ********************\n");
    return OK_RC;
}

//
// Test2 tests adding a few records to a file.
//
RC Test2(void) {
    RC rc;
    RM_FileHandle fh;

    printf("test2 starting ****************\n");

    if ((rc = CreateFile((char *) FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char *) FILENAME, fh)) ||
        (rc = AddRecs(fh, FEW_RECS)) ||
        (rc = VerifyFile(fh, FEW_RECS)) ||
        (rc = CloseFile((char *) FILENAME, fh)))
        return (rc);

    LsFile((char *) FILENAME);

    if ((rc = DestroyFile((char *) FILENAME)))
        return (rc);

    printf("\ntest2 done ********************\n");
    return OK_RC;
}

//
// Test3 tests adding a large number of records to a file.
//
RC Test3(void) {
    RC rc;
    RM_FileHandle fh;

    printf("test3 starting ****************\n");

    if ((rc = CreateFile((char *) FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char *) FILENAME, fh)) ||
        (rc = AddRecs(fh, LOTS_OF_RECS)) ||
        (rc = VerifyFile(fh, LOTS_OF_RECS)) ||
        (rc = CloseFile((char *) FILENAME, fh)))
        return (rc);

    LsFile((char *) FILENAME);

    if ((rc = DestroyFile((char *) FILENAME)))
        return (rc);

    printf("\ntest3 done ********************\n");
    return OK_RC;
}

//
// Test4 tests scanning the record
//
RC Test4(void) {
    RC rc;
    RM_FileHandle fh;

    printf("test4 starting ****************\n");

    if ((rc = CreateFile((char *) FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char *) FILENAME, fh)) ||
        (rc = AddRecs(fh, FEW_RECS)) ||
        (rc = VerifyFile(fh, FEW_RECS)))
        return (rc);

    int numComp = 10;
    RM_FileScan scan;
    printf("scanning records whose num <= %d\n", numComp);
    TRY(scan.OpenScan(fh, INT, sizeof(int), offsetof(TestRec, num),
                      LT_OP, (void *) &numComp));
    {
        RC rc;
        int n = 0;
        RM_Record rec;
        while (true) {
            rc = scan.GetNextRec(rec);
            if (rc == RM_EOF) {
                break;
            } else if (rc != 0) {
                return rc;
            }
            ++n;
            char *data;
            rec.GetData(data);
            assert(((TestRec *) data)->num < numComp);
        }
        printf("%d records found.\n", n);
        assert(n == numComp);
    }

    if ((rc = CloseFile((char *) FILENAME, fh)) ||
        (rc = DestroyFile((char *) FILENAME)))
        return (rc);

    printf("\ntest4 done ********************\n");
    return OK_RC;
}

//
// Test5 tests updating some records
//
RC Test5(void) {
    RC rc;
    RM_FileHandle fh;

    printf("test5 starting ****************\n");

    if ((rc = CreateFile((char *) FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char *) FILENAME, fh)) ||
        (rc = AddRecs(fh, FEW_RECS)) ||
        (rc = VerifyFile(fh, FEW_RECS)))
        return (rc);

    RM_Record rec;
    RM_FileScan sc;

    TRY(sc.OpenScan(fh, INT, sizeof(int), 0, NO_OP, NULL));
    for (rc = sc.GetNextRec(rec); rc != RM_EOF; rc = sc.GetNextRec(rec)) {
        if (rc) {
            return rc;
        }
        TestRec *data;
        rec.GetData(CVOID(data));
        data->num++;
        TRY(fh.UpdateRec(rec));
    }
    TRY(sc.CloseScan());

    TRY(sc.OpenScan(fh, INT, sizeof(int), 0, NO_OP, NULL));
    for (rc = sc.GetNextRec(rec); rc != RM_EOF; rc = sc.GetNextRec(rec)) {
        if (rc) {
            return rc;
        }
        TestRec *data;
        rec.GetData(CVOID(data));
        int old_num;
        sscanf(data->str, "a%d", &old_num);
        assert(old_num + 1 == data->num);
    }
    TRY(sc.CloseScan());

    if ((rc = CloseFile((char *) FILENAME, fh)) ||
        (rc = DestroyFile((char *) FILENAME)))
        return (rc);

    printf("\ntest5 done ********************\n");
    return OK_RC;
}

//
// Test6 tests updating some records
//
RC Test6(void) {
    RC rc;
    RM_FileHandle fh;

    printf("test6 starting ****************\n");

    if ((rc = CreateFile((char *) FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char *) FILENAME, fh)) ||
        (rc = AddRecs(fh, FEW_RECS)) ||
        (rc = VerifyFile(fh, FEW_RECS)))
        return (rc);

    RM_Record rec;
    RM_FileScan sc;

    char searchStr[] = {"a8"};

    // delete the record whose str = searchStr
    TRY(sc.OpenScan(fh, STRING, strlen(searchStr), offsetof(TestRec, str),
                    EQ_OP, searchStr));
    TRY(sc.GetNextRec(rec));
    assert(sc.GetNextRec(rec) == RM_EOF);
    TRY(sc.CloseScan());

    RM_RID RM_RID;
    TRY(rec.GetRid(RM_RID));
    TRY(fh.DeleteRec(RM_RID));

    TRY(sc.OpenScan(fh, STRING, strlen(searchStr), offsetof(TestRec, str),
                    EQ_OP, searchStr));
    assert(sc.GetNextRec(rec) == RM_EOF);
    TRY(sc.CloseScan());

    if ((rc = CloseFile((char *) FILENAME, fh)) ||
        (rc = DestroyFile((char *) FILENAME)))
        return (rc);

    printf("\ntest6 done ********************\n");
    return OK_RC;
}

