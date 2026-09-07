🟢 Matrix Terminal (60 FPS)
Uma implementação em C++ de alto desempenho do famoso efeito de Chuva Digital do Matrix criada exclusivamente para sistemas Linux. O projeto utiliza sequências de escape ANSI para renderização suave e raw mode para controle de teclado em tempo real.

🐧 Nota: Este programa é exclusivo para Linux (requer suporte a POSIX e emuladores de terminal compatíveis).

⚡ Como Usar (Sem Instalação)
Você não precisa compilar nem instalar nada! O executável matrix_terminal já vem pronto para uso.

Você pode abri-lo de duas formas no Linux:

1 - Pelo Terminal:

Abra o terminal na pasta do arquivo e rode:
chmod +x matrix_terminal && ./matrix_terminal

2 - Duplo Clique:

Garanta que o arquivo tenha permissão de execução (clique com o botão direito no arquivo matrix_terminal > Propriedades > Permissões > Permitir execução) e dê duplo clique no arquivo para abrir a aplicação.

🚀 FuncionalidadesExclusivo para Linux: Desenvolvido nativamente para terminais POSIX/Linux.60 FPS Fluídos: Animação renderizada com controle preciso de tempo via std::chrono.  Cores Dinâmicas (256 Cores): O topo de cada coluna brilha em branco reluzente, seguido por um degradê em tons de verde.  Redimensionamento Automático: Ajusta a animação em tempo real caso a janela do terminal mude de tamanho (SIGWINCH).  Abertura em Tela Cheia: Abre automaticamente em um terminal maximizado (gnome-terminal, konsole, mate-terminal ou xfce4-terminal).  Controle Fácil: Encerre o programa a qualquer momento pressionando q, Q ou ESC.  


🛠️ Compilação (Opcional)
Se você quiser compilar o código-fonte (matrix_terminal_60fps.cpp) manualmente:

Pré-requisitos
Distribuição Linux com compilador C++ com suporte a C++11 ou superior (ex: g++ ou clang++).

Passos
Bash
# Clonar o repositório
git clone https://github.com/seu-usuario/matrix-terminal.git
cd matrix-terminal

# Compilar
g++ -O3 matrix_terminal_60fps.cpp -o matrix_terminal

# Executar diretamente na janela atual do terminal
./matrix_terminal --rain
