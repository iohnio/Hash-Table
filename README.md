# Hash-Table
## Project Description
Hash-Table is a C++ project that implements a simplified file system using a hash table to efficiently manage file operations such as insertion, lookup, and deletion. File names are used as keys, and collisions are handled using linear probing, quadratic probing, and double hashing. The system dynamically resizes based on load factor and deletion ratio, performing incremental rehashing into a larger prime-sized table while excluding deleted entries. It also supports switching collision resolution strategies during runtime, with new policies applied after rehashing. The project emphasizes efficient storage management, collision handling, and adaptive performance through dynamic resizing and probing techniques.
## How to Run the Project
1. open terminal
2. compile: g++ driver.cpp filesys.cpp -o filesys
3. run the executable: ./filesys

## Example Usage
```bash
> insert test.cpp 252199
> insert test.cpp 337242
> insert info.txt 651273
> insert driver.cpp 127554

> dump
[4]  : test.cpp (252199, 1)
[11] : test.cpp (337242, 1)
[2]  : info.txt (651273, 1)
[0]  : driver.cpp (127554, 1)

> getFile test.cpp 337242
test.cpp (337242, 1)

> getFile missing.txt 123456
File not found
```
