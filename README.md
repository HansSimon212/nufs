# NUFS: A simple filesystem #
An implementation of a basic filesystem written using the
<a href=http://libfuse.github.io/doxygen/ target="_blank">FUSE API</a>. The primary purpose of this repository
is to display my best coding practices (commenting/style) and ability to write complex code.

<hr>
<h3>Directions for launching filesystem:</h3>
<hr>
<ol>
 <li>Launch any version of Debian/Ubuntu (or any other local modern Linux).</li>
 <li>Make sure your working directory is a proper Linux filesystem, not a remote-mounted Windows or Mac directory. WSL 2 (Windows Subsystem for Linux 2) works for this, while WSL 1 does not. <a href="https://www.youtube.com/watch?v=_fntjriRe48">If you are using Windows and a Linux machine is not available to you, follow this guide on how to install WSL 2.</a></li>
 <li>Clone this GitHub repository (HansSimon212/nufs) to your preferred location, (this is where the filesystem will be mounted).</li>
 <li>Make sure you have the following packages installed. Each can be installed with 'apt-get install packagename':
   <ul>
      <li>gcc</li>
      <li>pkg-config</li>
      <li>make</li>
      <li>libbsd-dev</li>
      <li>fuse</li>
      <li>libfuse-dev</li>
   </ul>
   </li>
 <li>In the directory where you cloned the project, enter 'make clean' to prepare for mounting.</li>
 <li>In the same directory, enter 'make mount' to mount NUFS (file system).</li>
 <li>Navigate to the 'mnt' folder. This is the root folder of the filesystem.</li>
</ol>
<br>

Now you can use those standard Linux filesystem commands (mkdir/rename/chmod/etc.) listed at the bottom of the 'nufs.c' file in this repository.

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
 <li>This project was an assignment for CS 3650: Computer Systems.  I strongly discourage plagiarism of this code as the course staff runs an automatic checker that compares code to all Bottlenose code, where this code was submitted, (and for the obvious reason that plagiarizing code is unwise/unethical and goes against the Academic Integrity codes of Northeastern University).</li>
 </b>
</ul>

