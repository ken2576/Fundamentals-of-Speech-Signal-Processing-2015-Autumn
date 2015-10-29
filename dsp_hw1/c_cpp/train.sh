read -p "Input how many iterations for seq1:  " iterations1
read -p "Input how many iterations for seq2:  " iterations2
read -p "Input how many iterations for seq3:  " iterations3
read -p "Input how many iterations for seq4:  " iterations4
read -p "Input how many iterations for seq5:  " iterations5

echo "Initiating..."
./optrain ${iterations1} model_init.txt seq_model_01.txt model_01.txt &
./optrain ${iterations2} model_init.txt seq_model_02.txt model_02.txt &
./optrain ${iterations3} model_init.txt seq_model_03.txt model_03.txt &
./optrain ${iterations4} model_init.txt seq_model_04.txt model_04.txt &
./optrain ${iterations5} model_init.txt seq_model_05.txt model_05.txt &

wait
echo "Training finished!"

./optest modellist.txt testing_data1.txt result.txt testing_answer.txt &
