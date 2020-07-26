#!/usr/bin/perl
use 5.16.0;
use warnings FATAL => 'all';

use Test::Simple tests => 28;
use IO::Handle;

sub mount {
    system("(make mount 2>&1) >> test.log &");
    sleep 1;
}

sub unmount {
    system("(make unmount 2>&1) >> test.log");
}

sub write_text {
    my ($name, $data) = @_;
    open my $fh, ">", "mnt/$name" or return;
    $fh->say($data);
    close $fh;
}

sub read_text {
    my ($name) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    local $/ = undef;
    my $data = <$fh> || "";
    close $fh;
    $data =~ s/\s*$//;
    return $data;
}

sub read_text_slice {
    my ($name, $count, $offset) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    my $data;
    seek $fh, $offset, 0;
    read $fh, $data, $count;
    close $fh;
    return $data;
}

system("rm -f data.nufs test.log");

say "#           == Basic Tests ==";
mount();

my $part1 = 0;

sub p1ok {
    my ($cond, $msg) = @_;
    if ($cond) {
        ++$part1;
    }
    else {
        ok(0, $msg);
    }
}

my $msg0 = "hello, one";
write_text("one.txt", $msg0);
ok(-e "mnt/one.txt", "File1 exists.");
p1ok(-f "mnt/one.txt", "File1 is regular file.");
my $msg1 = read_text("one.txt");
say "# '$msg0' eq '$msg1'?";
p1ok($msg0 eq $msg1, "read back data1");

my $msg2 = "hello, two";
write_text("two.txt", $msg2);
p1ok(-e "mnt/two.txt", "File2 exists.");
p1ok(-f "mnt/two.txt", "File2 is regular file.");
my $msg3 = read_text("two.txt");
say "# '$msg0' eq '$msg1'?";
p1ok($msg2 eq $msg3, "Read back data2 correctly.");

my $files = `ls mnt`;
p1ok($files =~ /one\.txt/, "one.txt is in the directory");
p1ok($files =~ /two\.txt/, "two.txt is in the directory");

my $long0 = "=This string is fourty characters long.=" x 50;
write_text("2k.txt", $long0);
my $long1 = read_text("2k.txt");
p1ok($long0 eq $long1, "Read back long correctly.");

my $long2 = read_text_slice("2k.txt", 10, 50);
my $right = "ng is four";
p1ok($long2 eq $right, "Read with offset & length");

say "# part1 = $part1";
ok($part1 == 9, "No regressions on the easy stuff.");

unmount();

ok(!-e "mnt/one.txt", "one.txt doesn't exist after umount");
$files = `ls mnt`;
ok($files !~ /one\.txt/, "one.txt is not in the directory");
ok($files !~ /two\.txt/, "two.txt is not in the directory");

mount();

$files = `ls mnt`;
ok($files =~ /one\.txt/, "one.txt is in the directory still");
ok($files =~ /two\.txt/, "two.txt is in the directory still");

$msg1 = read_text("one.txt");
say "# '$msg0' eq '$msg1'?";
ok($msg0 eq $msg1, "Read back data1 correctly again.");

$msg3 = read_text("two.txt");
say "# '$msg2' eq '$msg3'?";
ok($msg2 eq $msg3, "Read back data2 correctly again.");

system("rm -f mnt/one.txt");
$files = `ls mnt`;
ok($files !~ /one\.txt/, "deleted one.txt");

system("mv mnt/two.txt mnt/abc.txt");
$files = `ls mnt`;
ok($files !~ /two\.txt/, "moved two.txt");
ok($files =~ /abc\.txt/, "have abc.txt");

my $msg4 = read_text("abc.txt");
say "# '$msg2' eq '$msg4'?";
ok($msg2 eq $msg4, "Read back data after rename.");

say "#           == Less Basic Tests ==";

system("ln mnt/abc.txt mnt/def.txt");
my $msg5 = read_text("def.txt");
say "# '$msg2' eq '$msg5'?";
ok($msg2 eq $msg5, "Read back data after link.");

system("rm -f mnt/abc.txt");
my $msg6 = read_text("def.txt");
say "# '$msg2' eq '$msg6'?";
ok($msg2 eq $msg6, "Read back data after other link deleted.");

system("mkdir mnt/foo");
ok(-d "mnt/foo", "Made a directory");

system("cp mnt/def.txt mnt/foo/abc.txt");
my $msg7 = read_text("foo/abc.txt");
say "# '$msg2' eq '$msg7'?";
ok($msg2 eq $msg7, "Read back data from copy in subdir.");

my $huge0 = "=This string is fourty characters long.=" x 1000;
write_text("40k.txt", $huge0);
my $huge1 = read_text("40k.txt");
ok($huge0 eq $huge1, "Read back 40k correctly.");

my $huge2 = read_text_slice("40k.txt", 10, 8050);
$right = "ng is four";
ok($huge2 eq $right, "Read with offset & length");

system("mkdir -p mnt/dir1/dir2/dir3/dir4/dir5");
my $hi0 = "hello there";
write_text("dir1/dir2/dir3/dir4/dir5/hello.txt", $hi0);
my $hi1 = read_text("dir1/dir2/dir3/dir4/dir5/hello.txt");
ok($hi0 eq $hi1, "nested directories");

system("mkdir mnt/numbers");
for my $ii (1..50) {
    write_text("numbers/$ii.num", "$ii");
}

my $nn = `ls mnt/numbers | wc -l`;
ok($nn == 50, "created 50 files");

for my $ii (1..5) {
    my $xx = $ii * 10;
    my $yy = read_text("numbers/$xx.num");
    ok($yy =~ /^\d+$/ && $xx == $yy, "check value $xx");
}

for my $ii (1..4) {
    my $xx = $ii * 7;
    system("rm mnt/numbers/$xx.num");
}

unmount();

ok(!-d "mnt/numbers", "numbers dir doesn't exist after umount");

mount();

my $mm = `ls mnt/numbers | wc -l`;
ok($mm == 46, "deleted 4 files");

unmount();
