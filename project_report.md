# Project Report - Write xor Execute
2023–07-XX

Zacharias Terdin (<zacte@kth.se>)

Elin Kårehagen (<elikare@kth.se>)

## Report instruction:

The report should be ~ 5 pages and:
- Explain the countered vulnerabilities 						
- Explain you design choice
- Discuss you implementation details that you think are important
- Explain why software counter the selected attacks
- Explain your own contribution
(the first four parts can be the same for all members of the group, the section "explain your contribution" must be individual and consists of one page)


## Security Issue
Almost all software contains bugs and one of the most, if not the most, common type of software bug withing computer security is related to memory vulnerabilities.  Memory corruption, code corruption, control-flow hijacking are some examples of attacks which would allow an attacker to influence a system partially or completely. Fortunately, there exists protective mechanisms to counter these well-known attacks.

This project will be covering the prevention of code injection and execution of arbitrary code on s3k. The problem to be solved consists of two parts, a malicious actor can write and execute arbitrary code if:
1.	a program can write and execute code from the memory intended for program data.
2.	a program can write code into the section of memory intended for instructions, i.e., the program can rewrite its own code.

## Solution

This project aims to solve the problem by implementing the write xor execute policy. This is achieved by combining code integrity policy which enforces that memory intended for program code cannot be writable and the non-executable data policy which enforces that memory intended for program data (such as stack and heap) cannot be executable. This creates resilience towards code injection and arbitrary code execution since one piece of memory is always either writable or executable, and never both at once.

Software updates, however, might be a legitimate reason to rewrite program code. This action will be allowed after a method of authentication. The authentication will initially check that the program is trusted, and then during runtime authenticate the code if the program requests a change of privileges.


## Limitations


## Design choices
### Monitor
### Authentication
The technique implemented for generating signatures is by cipher block chaining message authentication code (cbc-mac), and the encryption method used in the block chaining is symmetric aes128. This method was not chosen because it is the most secure or best suited for the purpose, but because it was provided to us as a finished library and made implementing the concept of code authentication simpler. The scenario we have assumed present is that the monitor and distributor of the application share a secret key.

The code authentication is done before the application setup and every time the monitor changes the privileges of a memory segment from writable to executable. This is to ensure that the policy of code integrity is properly enforced. The process of authentication can be described with the following three steps:

1.	The distributor calculates the cbc-mac of the code with the secret key, and formats the code in such a way that the initial 128b of the code contains the signature.
2.	Before loading and setting up the app, the monitor calculates the cbc-mac of the code provided and matches it towards the one provided at distribution. Upon success the monitor starts setting up the app and if the authentication turns out unsuccessfull the setup is aborted.
3. If the app requires more memory during runtime, for example software updates, this can be done after another authentication of the new piece of code.

To make the process of authentication as seemless and realistic as possible a custom file format has been created. As mentioned earlier the signature is stored in the first 16 B of the file. The signature is then followed by specific information for each section of the program, this is so the monitor can set up the right memory capabilites before running. The header section info covers text, data, rodata, bss, heap and stack. This information is parsed from app.elf by a python script getsections.py with the help of pyelftools. The sections info is then read by app_format.c which puts together a final file with the complete header. 


A map over the file header:
<style>
    table, th, td {
    border: 1px solid black;
    border-collapse: collapse;
    table-layout: fixed;
    width: 400px;
    }
</style>
<table >
    <tr>
        <td colspan="2"> signature 16 B</td>
    </tr>
    <tr>
        <td>section name 8 B</td>
        <td>section address 8 B</td>
    </tr>
    <tr>
        <td>section size 8 B</td>
        <td>section permissions 8 B</td>
    <tr>
        <td colspan="2"> sections info repeated for all sections...</td>
    </tr>
    <tr>
        <td colspan="2"> bloat until end of header</td>
    </tr>
    </tr>
    
</table>

## Implementation details


## Individual contributions
### Elin Kårehagen
Due to joining the course later than Zacharias and lacking some of the required prior knowledge, mostly programing in C, I early on felt it was hard to keep up and make a meaningful contribution. This led to us splitting the project into two separate but still very intertwined branches. I have focused on the process of code authentication. As described above the ARS128 library was provided to us as a resource for this project and a natural next step in this project would probably be to expand to asymmetric encryption. However I perceived this as too big of a project for me, so instead I continued with creating the file format for the application which was more of a challange in my degree of difficulty. The pieces of code I have produced for this project are genpayload.py, app_format.c and the small part in the monitor handling the process of authentication. I have also been lead on structuring this report. 

### Zacharias Terdin


## References
