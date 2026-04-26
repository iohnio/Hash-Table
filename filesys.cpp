// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"

FileSys :: FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY) {
    // check valid size
    if (size < MINPRIME) {
        size = MINPRIME;
    } else if (size > MAXPRIME) {
        size = MAXPRIME;
    } else if (!isPrime(size)) {
        size = findNextPrime(size);
    }

    m_currentCap = size;
    m_currProbing = probing;
    m_hash = hash;

    // create memory for current hashtable
    m_currentTable = new File*[m_currentCap];
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }

    m_currentSize = 0;
    m_currNumDeleted = 0;
    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    m_oldProbing = probing;
    m_transferIndex = 0;
}

FileSys :: ~FileSys() {
    // free the files in m_currentTable
    if (m_currentTable != nullptr) {
        for (int i = 0; i < m_currentCap; i++) {
            delete m_currentTable[i];
        }
        delete[] m_currentTable;
    }
    // free the files in m_oldTable
    if (m_oldTable != nullptr) {
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];
        }
        delete[] m_oldTable;
    }
}

void FileSys :: changeProbPolicy(prob_t policy) {
    m_newPolicy = policy;
}

bool FileSys :: insert(File file) {
    // check valid file block number
    if (file.getDiskBlock() < DISKMIN || file.getDiskBlock() > DISKMAX) {
        return false;
    }
    // perform incremental rehashing if an old table exists
    if (m_oldTable != nullptr) {
        int portionSize = m_oldCap / 4;
        int endIndex = min(m_transferIndex + portionSize, m_oldCap);
        for (int i = m_transferIndex; i < endIndex; i++) {
            if (m_oldTable[i] != nullptr) {
                int newIndex = m_hash(m_oldTable[i]->getName()) % m_currentCap;
                int probeCount = 0;
                while (m_currentTable[newIndex] != nullptr) {
                    newIndex = getNextIndex(newIndex, ++probeCount, m_oldTable[i]->getName());
                }
                // transfer file from old table to current table
                m_currentTable[newIndex] = m_oldTable[i];
                m_oldTable[i] = nullptr;
            }
        }
        m_transferIndex = endIndex;
        // check if rehashing is complete
        if (m_transferIndex == m_oldCap) {
            delete[] m_oldTable;
            m_oldTable = nullptr;
            m_oldCap = 0;
            m_transferIndex = 0;
        }
    }
    // check for duplicates in the current table
    int index = m_hash(file.getName()) % m_currentCap;
    int probeCount = 0;
    while (m_currentTable[index] != nullptr) {
        if (*m_currentTable[index] == file) {
            return false;
        }
        index = getNextIndex(index, ++probeCount, file.getName());
    }
    // insert the file into the current table
    m_currentTable[index] = new File(file);
    m_currentSize++;
    // check if rehashing is needed after insertion
    if (lambda() > 0.5) {
        int newCapacity = findNextPrime(4 * (m_currentSize - m_currNumDeleted));
        m_oldTable = m_currentTable;
        m_oldCap = m_currentCap;
        m_currentCap = newCapacity;
        m_currentTable = new File*[m_currentCap]();
        m_transferIndex = 0;
    }
    return true;
}

bool FileSys :: remove(File file) {
    int index = m_hash(file.getName()) % m_currentCap;
    int probeCount = 0;
    // search for the file in the table using probing
    while (m_currentTable[index] != nullptr) {
        // if the file is found and matches both the name and disk block, mark as deleted
        if (m_currentTable[index]->getName() == file.getName() &&
            m_currentTable[index]->getDiskBlock() == file.getDiskBlock() &&
            m_currentTable[index]->getUsed()) {

            m_currentTable[index]->setUsed(false);
            m_currNumDeleted++;
            m_currentSize--;

            delete m_currentTable[index];
            m_currentTable[index] = nullptr;

            // if more than 80% of the occupied buckets are deleted, trigger rehashing
            if (deletedRatio() > 0.8) {
                int newCapacity = findNextPrime(4 * (m_currentSize - m_currNumDeleted));
                File** newTable = new File*[newCapacity]();
                // rehash all live data from the current table
                for (int i = 0; i < m_currentCap; i++) {
                    if (m_currentTable[i] != nullptr && m_currentTable[i]->getUsed()) {
                        int newIndex = m_hash(m_currentTable[i]->getName()) % newCapacity;
                        int probeCount = 0;
                        while (newTable[newIndex] != nullptr) {
                            newIndex = getNextIndex(newIndex, ++probeCount, m_currentTable[i]->getName());
                        }
                        newTable[newIndex] = m_currentTable[i];
                    }
                }
                // cleanup old table and free memory for files that are not used anymore
                for (int i = 0; i < m_currentCap; i++) {
                    if (m_currentTable[i] != nullptr && !m_currentTable[i]->getUsed()) {
                        delete m_currentTable[i];
                    }
                }
                delete[] m_currentTable;
                m_currentTable = newTable;
                m_currentCap = newCapacity;
            }
            return true;
        }
        // continue probing if not found
        index = getNextIndex(index, ++probeCount, file.getName());
    }
    return false;
}

const File FileSys :: getFile(string name, int block) const{
    int index = m_hash(name) % m_currentCap;
    int probeCount = 0;

    while (m_currentTable[index] != nullptr) {
        if (m_currentTable[index]->getName() == name && m_currentTable[index]->getDiskBlock() == block) {
            return *m_currentTable[index];
        }
        index = getNextIndex(index, ++probeCount, name);
    }
    // return an empty object if file not found
    return File();
}

bool FileSys :: updateDiskBlock(File file, int block) {
    int index = m_hash(file.getName()) % m_currentCap;
    int probeCount = 0;

    // search for the file using probing
    while (m_currentTable[index] != nullptr) {
        if (m_currentTable[index]->getName() == file.getName() && 
            m_currentTable[index]->getDiskBlock() == file.getDiskBlock() && 
            m_currentTable[index]->getUsed()) {
            
            m_currentTable[index]->setDiskBlock(block);
            return true;
        }
        // continue probing if file is not found at current index
        index = getNextIndex(index, ++probeCount, file.getName());
    }
    // return false if the file is not found
    return false;
}


float FileSys :: lambda() const {
    return float(m_currentSize + m_currNumDeleted) / m_currentCap;
}

float FileSys :: deletedRatio() const {
    if (m_currentSize == 0) {
        return 0.0;
    }
    return float(m_currNumDeleted) / m_currentSize;
}

void FileSys::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool FileSys::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int FileSys :: findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

int FileSys :: getNextIndex(int index, int probeCount, const string &fileName) const {
    switch (m_currProbing) {
        case LINEAR:
            return (index + probeCount) % m_currentCap;
        case DOUBLEHASH: {
            int secondHash = 11 - (m_hash(fileName) % 11);
            return (index + probeCount * secondHash) % m_currentCap;
        }
        default:
            return (index + probeCount * probeCount) % m_currentCap;
    }
}

