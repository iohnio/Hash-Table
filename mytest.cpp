#include "filesys.h"
#include <random>
#include <algorithm>

using namespace std;

const char * FAIL_STATEMENT = "*****TEST FAILED: ";
const char * PASS_STATEMENT = "     TEST PASSED: ";

enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};

class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = std::mt19937(10);// 10 is the fixed seed value
        m_unidist = std::uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    int getMin(){return m_min;}
    int getMax(){return m_max;}
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

class Tester {
public:
    bool testConstructorNormalCase();
    bool testInsert();
    bool testRehashTriggered();
    bool testRehashCompletion();
    bool testGetFileErrorCase();
    bool testGetFileNonCollidingKeys();
    bool testGetFileCollidingKeys();
    bool testRemoveNonCollidingKeys();
    bool testRemoveCollidingKeys();
    bool testRehashTriggeredAfterRemove();
    bool testRehashCompletionAfterRemove();
    bool testUpdateDiskBlockNormalCase();
    bool testChangeProbPolicy();
};

int main() {
    Tester aTester;
    cout << endl;
    //Test 1
    cout << "Testing constructor normal case:" << endl;
    if (aTester.testConstructorNormalCase()) {
        cout << PASS_STATEMENT << "constructor initialized with valid parameters" << endl;
    } else {
        cout << FAIL_STATEMENT << "constructor failed to initialize with valid parameters" << endl;
    }
    cout << endl;
    //Test 2
    cout << "Testing insert function:" << endl;
    if (aTester.testInsert()) {
        cout << PASS_STATEMENT << "function inserted all files properly" << endl;
    } else {
        cout << FAIL_STATEMENT << "function did not insert all files properly" << endl;
    }
    cout << endl;
    //Test 3
    cout << "Testing rehash trigger:" << endl;
    if (aTester.testRehashTriggered()) {
        cout << PASS_STATEMENT << "rehashing is triggered after a decent number of insertions" << endl;
    } else {
        cout << FAIL_STATEMENT << "rehashing is not triggered after a decent number of insertions" << endl;
    }
    cout << endl;
    //Test 4
    cout << "Testing rehash completion:" << endl;
    if (aTester.testRehashCompletion()) {
        cout << PASS_STATEMENT << "all live data was transferred to the new table and the old table is removed" << endl;
    } else {
        cout << FAIL_STATEMENT << "all live data was not transferred to the new table and/or the old table is not removed" << endl;
    }
    cout << endl;
    //Test 5
    cout << "Testing getFile error case:" << endl;
    if (aTester.testGetFileErrorCase()) {
        cout << PASS_STATEMENT << "function returns empty file" << endl;
    } else {
        cout << FAIL_STATEMENT << "function does not return empty file" << endl;
    }
    cout << endl;
    //Test 6
    cout << "Testing getFile non-colliding keys:" << endl;
    if (aTester.testGetFileNonCollidingKeys()) {
        cout << PASS_STATEMENT << "function finds all non-colliding keys" << endl;
    } else {
        cout << FAIL_STATEMENT << "function does not find all non-colliding keys" << endl;
    }
    cout << endl;
    //Test 7
    cout << "Testing getFile colliding keys:" << endl;
    if (aTester.testGetFileCollidingKeys()) {
        cout << PASS_STATEMENT << "function finds all colliding keys" << endl;
    } else {
        cout << FAIL_STATEMENT << "function does not find all colliding keys" << endl;
    }
    cout << endl;
    //Test 8
    cout << "Testing remove non-colliding keys:" << endl;
    if (aTester.testRemoveNonCollidingKeys()) {
        cout << PASS_STATEMENT << "function removes all non-colliding keys" << endl;
    } else {
        cout << FAIL_STATEMENT << "function does not remove all non-colliding keys" << endl;
    }
    cout << endl;
    //Test 9
    cout << "Testing remove colliding keys:" << endl;
    if (aTester.testRemoveCollidingKeys()) {
        cout << PASS_STATEMENT << "function removes all colliding keys" << endl;
    } else {
        cout << FAIL_STATEMENT << "function does not remove all colliding keys" << endl;
    }
    cout << endl;
    //Test 10
    cout << "Testing rehash trigger after removal:" << endl;
    if (aTester.testRehashTriggeredAfterRemove()) {
        cout << PASS_STATEMENT << "rehashing is triggered after a decent number of removals" << endl;
    } else {
        cout << FAIL_STATEMENT << "rehashing is not triggered after a decent number of removals" << endl;
    }
    cout << endl;
    //Test 11
    cout << "Testing rehash completion after removal:" << endl;
    if (aTester.testRehashCompletionAfterRemove()) {
        cout << PASS_STATEMENT << "all live data was transferred to the new table and the old table is removed" << endl;
    } else {
        cout << FAIL_STATEMENT << "all live data was not transferred to the new table and/or the old table is not removed" << endl;
    }
    cout << endl;
    //Test 12
    cout << "Testing updateDiskBlock normal case:" << endl;
    if (aTester.testUpdateDiskBlockNormalCase()) {
        cout << PASS_STATEMENT << "updated disk block number" << endl;
    } else {
        cout << FAIL_STATEMENT << "did not update disk block number" << endl;
    }
    cout << endl;
    //Test 13
    cout << "Testing change policy:" << endl;
    if (aTester.testChangeProbPolicy()) {
        cout << PASS_STATEMENT << "changed probing policy after rehash" << endl;
    } else {
        cout << FAIL_STATEMENT << "did not change probing policy after rehash" << endl;
    }
    cout << endl;
}

//***********************************************
unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;
   for (unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val;
}

string namesDB[6] = {"driver.cpp", "test.cpp", "test.h", "info.txt", "mydocument.docx", "tempsheet.xlsx"};
//***********************************************

bool Tester :: testConstructorNormalCase() {
    int expectedSize = MINPRIME;
    hash_fn expectedHashFunction = hashCode;
    prob_t expectedProbing = QUADRATIC;

    FileSys filesys(MINPRIME, hashCode, QUADRATIC);

    if (filesys.m_currentCap != expectedSize) {
        return false;
    }
    if (filesys.m_currProbing != expectedProbing) {
        return false;
    }
    if (filesys.m_currentTable == nullptr) {
        return false;
    }
    if (filesys.m_currentSize != 0) {
        return false;
    }
    if (filesys.m_currNumDeleted != 0) {
        return false;
    }
    if (filesys.m_oldTable != nullptr) {
        return false;
    }
    if (filesys.m_oldCap != 0) {
        return false;
    }
    if (filesys.m_oldSize != 0) {
        return false;
    }
    if (filesys.m_oldNumDeleted != 0) {
        return false;
    }
    if (filesys.m_transferIndex != 0) {
        return false;
    }

    return true;
}

bool Tester :: testInsert() {
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, DOUBLEHASH);
    bool result = true;
    
    for (int i = 0; i < 49; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        if (!filesys.insert(dataObj)) {
            cout << "Did not insert " << &dataObj << endl;
            result = false;
        }
    }
    return result && (filesys.m_currentSize == 49);
}

bool Tester :: testRehashTriggered() {
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, DOUBLEHASH);
    int rehashThreshold = 51; // must be greater than 101 * 0.5 = 50.5
    float lambda50 = -1.0, lambda51 = -1.0;  // to store load factor after 50th and 51st insertions
    bool rehashed = false;

    for (int i = 0; i < 100; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        if (!filesys.insert(dataObj)) {
            cout << "Did not insert " << &dataObj << endl;
            return false;
        }
        if (i == 49) {
            lambda50 = filesys.lambda();
        }
        if (i == 50) {
            lambda51 = filesys.lambda();
        }
    }
    if (filesys.m_currentSize > rehashThreshold && lambda50 > lambda51) {
        rehashed = true;
    }

    return rehashed;
}

bool Tester :: testRehashCompletion() {
    Random RndID(DISKMIN, DISKMAX);
    Random RndName(0, 5);
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    
    int rehashThreshold = 51;
    bool rehashed = false;
    int initialTableSize = filesys.m_currentCap;
    int initialOldTableSize = filesys.m_oldCap;

    for (int i = 0; i < 100; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        if (!filesys.insert(dataObj)) {
            cout << "Did not insert " << &dataObj << endl;
            return false;
        }
    }
    // check that rehashing was triggered by the load factor
    if (filesys.m_currentSize > rehashThreshold && filesys.lambda() < 0.5) {
        rehashed = true;
    }
    // check if data is transferred to the new table after rehash
    bool dataTransferred = true;
    for (int i = 0; i < 100; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        bool foundInNewTable = false;
        for (int j = 0; j < filesys.m_currentCap; j++) {
            if (filesys.m_currentTable[j] != nullptr &&
                filesys.m_currentTable[j]->getName() == dataObj.getName()) {
                foundInNewTable = true;
            }
        }
        if (!foundInNewTable) {
            dataTransferred = false;
        }
    }
    // check if the old table is properly removed
    bool oldTableRemoved = (filesys.m_oldCap == 0);

    if (rehashed && dataTransferred && oldTableRemoved) {
        return true;
    } else {
        return false;
    }
}

bool Tester :: testGetFileErrorCase() {
    FileSys filesys(MAXPRIME, hashCode, QUADRATIC);
    string fileName = "error";
    int fileBlock = DISKMIN - 1;

    File result = filesys.getFile(fileName, fileBlock);

    // check that the result is an empty File object
    if (result.getName() == "" && result.getDiskBlock() == 0) {
        return true;
    } else {
        return false;
    }
}

bool Tester :: testGetFileNonCollidingKeys() {
    FileSys filesys(MAXPRIME, hashCode, LINEAR);

    const int count = 50;
    string baseName = "file";
    int baseBlock = DISKMIN;

    // insert several files with unique names and disk blocks
    for (int i = 0; i < count; i++) {
        string fileName = baseName + to_string(i);
        int fileBlock = baseBlock + i;
        File file(fileName, fileBlock);
        filesys.insert(file);
    }
    // retrieve and validate each file
    for (int i = 0; i < count; i++) {
        string fileName = baseName + to_string(i);
        int fileBlock = baseBlock + i;
        File result = filesys.getFile(fileName, fileBlock);

        // check that the retrieved file matches the inserted file
        if (result.getName() != fileName || result.getDiskBlock() != fileBlock) {
            return false;
        }
    }

    return true;
}

bool Tester :: testGetFileCollidingKeys() {
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, DOUBLEHASH);
    // insert the file into the system
    for (int i = 0; i < 49; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        if (!filesys.insert(dataObj)) {
            cout << "Did not insert " << &dataObj << endl;
            return false;
        }
    // retrieve the file
    File result = filesys.getFile(dataObj.getName(), dataObj.getDiskBlock());
    if (result.getName() != dataObj.getName() || result.getDiskBlock() != dataObj.getDiskBlock()) {
        cout << "failed to retrieve file with name: " << dataObj.getName() << " and block: " << dataObj.getDiskBlock() << endl;
        return false;
        }
    }
    return true;
}

bool Tester :: testRemoveNonCollidingKeys() {
    FileSys filesys(MINPRIME, hashCode, LINEAR);
    // insert 49 files into the system with non-colliding names and disk blocks
    for (int i = 0; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File dataObj(fileName, diskBlock, true);
        if (!filesys.insert(dataObj)) {
            cout << "Did not insert " << fileName << endl;
            return false;
        }
    }
    // remove files from the system
    for (int i = 0; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);
        if (!filesys.remove(file)) {
            cout << "Remove failed for " << fileName << endl;
            return false;
        }
    }
    // verify the files were removed correctly
    for (int i = 0; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);
        File result = filesys.getFile(file.getName(), file.getDiskBlock());
        // file should not exist after removal
        if (result.getName() != "" || result.getDiskBlock() != 0) {
            cout << "File not removed correctly: " << fileName << endl;
            return false;
        }
    }
    return true;
}

bool Tester :: testRemoveCollidingKeys() {
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    // insert 12 files with colliding keys
    for (int i = 0; i < 12; i++) {
        string fileName = "file" + to_string(i%10);
        int diskBlock = DISKMIN + i;  // Unique disk blocks for each file
        File dataObj(fileName, diskBlock, true);
        if (!filesys.insert(dataObj)) {
            cout << "did not insert " << fileName << " with disk block " << diskBlock << endl;
            return false;
        }
    }
    // remove files from the system (these files should collide in the hash table)
    for (int i = 0; i < 12; i++) {
        string fileName = "file" + to_string(i%10);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);
        if (!filesys.remove(file)) {
            cout << "remove failed for " << fileName << " with disk block " << diskBlock << endl;
            return false;
        }
    }
    // verify the files were removed correctly
    for (int i = 0; i < 12; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);
        File result = filesys.getFile(file.getName(), file.getDiskBlock());
        // file should not exist after removal
        if (result.getName() != "" || result.getDiskBlock() != 0) {
            cout << "file not removed correctly: " << fileName << " with disk block " << diskBlock << endl;
            return false;
        }
    }
    return true;
}

bool Tester :: testRehashTriggeredAfterRemove() {
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    // insert enough files to trigger rehashing during removal
    for (int i = 0; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File dataObj(fileName, diskBlock, true);
        if (!filesys.insert(dataObj)) {
            cout << "Did not insert " << fileName << endl;
            return false;
        }
    }
    int initialSize = filesys.m_currentSize;
    // remove enough files to trigger rehashing
    int numRemoved = 0;
    for (int i = 0; i < 40; i++) {  
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);

        if (!filesys.remove(file)) {
            cout << "Remove failed for " << fileName << endl;
            return false;
        }
        numRemoved++;
    }
    int resultSize = filesys.m_currentSize;

    if (initialSize <= resultSize) {
        return false;
    }
    // check if the remaining files are still correctly retrievable
    for (int i = 40; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);
        File result = filesys.getFile(file.getName(), file.getDiskBlock());
        if (result.getName() != fileName || result.getDiskBlock() != diskBlock) {
            cout << "File not found after rehashing: " << fileName << endl;
            return false;
        }
    }
    return true;
}

bool Tester :: testRehashCompletionAfterRemove() {
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    // insert enough files to trigger rehashing during removal
    for (int i = 0; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File dataObj(fileName, diskBlock, true);
        if (!filesys.insert(dataObj)) {
            cout << "did not insert " << fileName << endl;
            return false;
        }
    }
    // remove enough files to trigger rehashing
    int numRemoved = 0;
    for (int i = 0; i < 40; i++) {  
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);

        if (!filesys.remove(file)) {
            cout << "remove failed for " << fileName << endl;
            return false;
        }
        numRemoved++;
    }
    if (!filesys.deletedRatio() > 0.8) {
        return false;
    }
    // check if the remaining files are still correctly retrievable
    for (int i = 40; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File file(fileName, diskBlock, true);
        File result = filesys.getFile(file.getName(), file.getDiskBlock());
        if (result.getName() != fileName || result.getDiskBlock() != diskBlock) {
            cout << "file not found after rehashing: " << fileName << endl;
            return false;
        }
    }
    return true;
}

bool Tester :: testUpdateDiskBlockNormalCase() {
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    File file("testfile", DISKMIN, true);

    if (!filesys.insert(file)) {
        cout << "Failed to insert file" << endl;
        return false;
    }
    // update disk block number
    int newBlock = DISKMAX;
    if (!filesys.updateDiskBlock(file, newBlock)) {
        cout << "failed to update disk block for file" << endl;
        return false;
    }

    if (!file.getDiskBlock() == newBlock) {
        cout << "failed to correctly update disk block for file" << endl;
        return false;
    }
    return true;
}

bool Tester :: testChangeProbPolicy() {
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    // insert some files
    for (int i = 0; i < 49; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File dataObj(fileName, diskBlock, true);
        if (!filesys.insert(dataObj)) {
            cout << "Failed to insert file " << fileName << endl;
            return false;
        }
    }
    filesys.changeProbPolicy(LINEAR);
    // insert more files to trigger rehashing
    for (int i = 49; i < 100; i++) {
        string fileName = "file" + to_string(i);
        int diskBlock = DISKMIN + i;
        File dataObj(fileName, diskBlock, true);
        if (!filesys.insert(dataObj)) {
            cout << "Failed to insert file " << fileName << endl;
            return false;
        }
    }
    // check if probing policy has been updated after the rehash
    if (!filesys.m_currProbing == LINEAR) {
        cout << "rehash failed to apply the new probing policy." << endl;
        return false;
    }
   
    return true;
}




