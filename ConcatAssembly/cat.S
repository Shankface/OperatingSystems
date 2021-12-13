_main:
.text                          

    .global _start            

_start:

read_write:                                  
	  movl    $4096,%edx           # third argument: message length
	  movl    $buf,%ecx           # second argument: pointer to buf to read
	  movl    $0,%ebx             # first argument: file descriptor (stdin)
	  movl    $3,%eax             # system call number 3 (sys_read)
	  int     $0x80               # call kernel

      cmpl    $0,%eax             # check to see if it read more than 0 bytes 
      jle     done				  # exit if it read <= 0 bytes
      
      movl    %eax,%edx           # third argument: length of bytes read from sys_read
	  movl    $buf,%ecx           # second argument: pointer to buf to write
	  movl    $1,%ebx             # first argument: file descriptor (stdout)
	  movl    $4,%eax             # system call number 4 (sys_write)
	  int     $0x80               # call kernel
                                  
      jmp     read_write		  # repeat process until 'ctrl d' is entered

done:
      movl    %eax,%ebx           # put return number from sys_read into %ebx
      movl    $1,%eax		      
      int     $0x80

.bss                           
    .comm     buf,4096,4
