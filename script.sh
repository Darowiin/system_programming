mkdir files
cd files
mkdir a_2
cd a_2
mkdir b_1 b_0
cd b_0
head -c 105 /dev/urandom > c_0.bin
head -c 673 /dev/urandom > c_1.bin
echo cat > c_2.txt
mkdir c_3
cd ../
ln b_0/c_0.bin b_2.bin
cd ../
cd ../
ln -s a_2/b_0 files/a_0
ln -s a_2/b_0/c_1.bin files/a_1.bin