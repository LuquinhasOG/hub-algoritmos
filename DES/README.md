# DES

Este projeto é uma implementação do Data Encryption Standard (DES), segundo especificações FIPS 46-3.

O executável gerado por este código recebe os seguintes argumentos:

* e: Define que será realizado a operação de criptografar;
* d: Define que será realizada a operação de descriptografar;
* -i: Seguido do arquivo de entrada (com extensão);
* -o: Seguido do arquivo de entrada (com extensão);
* -gK: Argumento opcional para gerar a chave antes de criptografar.

Exemplo 1: ./des.exe e -i lorem-ipsum.txt -o segredo.des -gK

Exemplo 2: ./des.exe d -i segredo.des -o mensagem.txt

No momento de criptografar e descriptografar é necessario que o arquivo .key esteja no mesmo diretório do arquivo de entrada. Após a operação finalizar, será gerado o arquivo de saída (junto do .key, caso usar o argumento -gK).

(Bug: Se o tamanho do arquivo, em bytes, não for múltiplo de 8, os últimos bytes que sobram não criptografam ou descriptografam corretamente.)
