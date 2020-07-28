# NUFS: A simple filesystem #
An implementation of a basic filesystem written using the
<a href=http://libfuse.github.io/doxygen/ target="_blank">FUSE API</a>. The primary purpose of this repository
is to display my best coding practices (commenting/style) and ability to write complex code.

<hr>
<h3>General:</h3>
<hr>
   
<h4>Languages/Technologies Used:</h4>
<ul>
 <li>C (for all functional code)</li>
 <li>FUSE: Filesystem in Userspace (API used to communicate with OS)</li>
 <li>Make (used in Makefile)</li>
 <li>Perl (test script provided in assignment statement)</li>
 <li>Ubuntu 18.04 (for development/testing/compilation)</li>
</ul>
 

<h4>Notes:</h4>
<ul>
 <b>
 <li>All code besides skeletons for several functions in 'nufs.c' was written by myself and my partner.</li>
 <li>This project was an assignment for CS 3650: Computer Systems.  I strongly discourage plagiarism of this code as the course staff runs an automatic checker to all Bottlenose code, where this code was submitted, (and for the obvious reason that plagiarizing code goes against the Academic Integrity codes of Northeastern University).</li>
 </b>
</ul>

<div>
</div>


<br>
<hr>
<h3>Directions for launching filesystem:</h3>
<hr>
<ol>
 <li>Launch any version of Debian/Ubuntu (or any other local modern Linux).</li>
 <li>Install fuse and libfuse-dev packages.</li>
 <li>Make sure your working directory is a proper Linux filesystem, not a remote-mounted Windows or Mac directory.</li>
 <li>Clone this GitHub repository (HansSimon212/nufs) to your preferred location.</li>
 <li>In your terminal, navigate to the directory where you want the filesystem to be mounted.</li>
 <li>Make sure you have the following packages installed:
   <ul>
      <li>gcc (GNU C Compiler) (downloaded with 'apt-get install gcc')</li>
      <li>pkg-config (downloaded with 'apt-get install pkg-config')</li>
      <li>make (downloaded with 'apt-get install make')</li>
   </ul>
   </li>
 <li>In your chosen directory, enter 'make clean' to prepare directory for mounting.</li>
 <li>In the same directory, enter'make mount' to mount NUFS (file system).</li>
</ol>
<br>

Now you can use those standard Linux filesystem commands (mkdir/rename/chmod/etc.) listed at the bottom of the 'nufs.c' file in this repository.
