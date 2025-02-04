# woody-woodpacker
> This program is a basic code injection in a ELF 64 binary.


### Summary
- [Notions and theory](#notions-and-theory)
	- [Packers](#)
	- [Encryption](#encryption)
		- [Symmetrical encryption](#symmetrical-encryption)
		- [Asymmetrical encryption](#asymmetrical-encryption)
	- [64 bits ELF](#64-bits-elf)
	- [Encruption algorithms](#encryption-algorithms)
	- [Compression methods](#compression-methods)
- [Objectives](#objectives)


## Notions and theory
In this section we will describe and explain the differents notions and technologies described in the subject.

Notions and key words in the subject:
- Packers
- "[] Encrypt a program given as parameter. Only 64 bits ELF files wille be managed here"
- "It [the program] will have to be decrypted to be run"
- "You are free to choose the encryption algorithm"
- "[] We strongly advice to explore the possible [compression] methods"

### Packers
Packer is another name for **executable compressor**.<br>
"Executable compression is any means of compressing an executable file and combining the compressed data with decompression code into a single executable. When this compressed executable is executed, the decompression code recreates the original code from the compressed code before executing it." - Wikipedia.
Compression is used by software distibutors for **size constraints** (distribution media as CD or floppy disks) or to **reduce bandwith** time the customer need to addess the software. It is also used to deter reverse engineering or **obfuscate** the content of the executable to hide malware for example. *The obfuscation part is the one that is important in our project.* 

Sources: [https://en.wikipedia.org/wiki/Executable_compression](https://en.wikipedia.org/wiki/Executable_compression)

### Encryption
Encryption consists in transforming raw text in an "encrypted" one, generally using algorithms (also called a cypher). Decryption requires an encryption key, string of digits or a password. Encryption ensures confidentiality (the data stays confidentials), integrity (check the origin of the data), authenticity (checks if data been modify since being sent), non-repudiation (prevent the expeditor to deny he is).

There is two types of encryption:<br>
- [Symmetrical](#symmetrical-encryption)
- [Asymmetrical](#asymmetrical-encryption)

#### Symmetrical encryption
The **same key** is used for the encryption and the decryption. These keys are considered cheaper to produce, encrypt and decrypt. Meaning that someone getting that key can decrypt all datas. The key has to be secured with another key. 

#### Asymmetrical encryption
Also called public key cryptography. It uses two disctint key to encrypt and decrypt the data. The **public key** shared between all for encryption. Any person having this key can encrypt a message but only the one having the **private key** can decrypt the data.


Source: [https://cloud.google.com/learn/what-is-encryption](https://cloud.google.com/learn/what-is-encryption)<br>
More about keys and encryption: [https://medium.com/codeclan/what-are-encryption-keys-and-how-do-they-work-cc48c3053bd6](https://medium.com/codeclan/what-are-encryption-keys-and-how-do-they-work-cc48c3053bd6)

### 64 bits ELF
Executable and Linkable Format (ELF) is a binary file format used to store compiled code: for example files '.o', '.so' or '.exe'. It is used in most Operating System except MacOS X. The file is built with a fixed header, segments and sections. Segments contain necessary informations to the execution and sections contains informations to link the program and replace data as variables.

Source: [https://fr.wikipedia.org/wiki/Executable_and_Linkable_Format](https://fr.wikipedia.org/wiki/Executable_and_Linkable_Format)

### Encryption algorithms
As **popular encryption algorithms** we can list:
- **Triple DES**: symmetrical
- **AES**: impervious to all attacks except for brut force
- **RSA Security**: asymmetrical, standard for encrypting data sent over the internet
- **Blowfish**: symmetrical, quick and effective. Used, for example, to secure payment on ecommerce platforms
- **Twofish**: symmetrical, fastest of its kind, ideal in hardware and software environments

Source: [https://www.arcserve.com/blog/5-common-encryption-algorithms-and-unbreakables-future](https://www.arcserve.com/blog/5-common-encryption-algorithms-and-unbreakables-future)

### Compression methods
"Data compression (also known as source coding or bit-rate reduction) modifies, encodes, or converts bits of data so it reduces the storage size, expediting data transmission and decreasing operational costs."<br>
**Data compression** can be **categorized** in two types:
- **lossy**: looses some data, commonly used for situation where the loss of quality is aceptable (JPEG or MP3 for example)
- **lossless**: retain all data, used for text documents or program files where all data is crucial

Example of **compression algotrithms**:
- **Huffman coding**: popular and efficient, it assignes short codes for frequent data symbol and longer for less frequent ones
- **Run-Length Encoding (RLE)**: replace long sequence of the same value with the sequence and a count
- **Lempel-Ziv-Welch (LZW)**: used in GIF format, based on a dictionary. It is very efficient for repetitive patterns
- **Dictionary Coding**: larger category of data compression. It relies on a predefined dictionary containing common data patterns
- **Perceptual coding**: mainly used in video and audio encoding. It removes data that are not easily perceived by humans as repetitive patterns while maintaining an acceptable quality

Source: [https://www.seagate.com/fr/fr/blog/what-is-data-compression/](https://www.seagate.com/fr/fr/blog/what-is-data-compression/)

## Objectives
The objective of the program is to **encrypt** a program given as parameter and **inject** code at the beginning of it. A **new executable** will generated. This new binary will have to be **decrypted** before execution and behave as the original program in the last step.

### Why encrypt?
From **attackers**, encryption permits to "**hindering** forensic analysis in case of detection, hindering copying of confidential data, adding functionality to the protected binary.<br>
From **defender**, adding a level of authorization checks, hindering analysis of customised intrusion detection tools , adding functionality to the protected binary.

In our case, we want our binary to be sure that only athorised individuals executes the binary and to obscure or hide the "new" binary and/or the way it works.<br>
For encryption we will use a very simple encryption method called the **XOR cipher**. This method consists by simply pass the bytes of the binary through a binary XOR operator with a fixed value.

## ELF infection
ELF infection consists, like the subject asks, in inserting code in an existing executable without altering its original behaviour. The memory of executables are organised as aligned pages. It basically means that in memory segments constituting pages that are not entirely filled with the text and data, padding will we added before or after data to fill them. We will use this padding to insert our code.<br>
Then to execute the parasite, the ELF header where information about the executable including the virtual address of the entrypoint. The parasite will then have to give the control back to the executable.

To do so, we will have to go from a section to another starting from the elf header. In our C program it can be done using structure that are predefined in elf.h.<br>
In this structure the usefull members are "e_entry" (gives vritual address of the program's entrypoint), "e_phoff" (gives the file offset for the start of the program header table).<br>
