# Publicar uma versão do MasterRGB

O código e as Releases ficam no mesmo repositório, `m1000v2/MasterRGB`. O workflow usa o `GITHUB_TOKEN` padrão do Actions, com permissão `contents: write`; não é necessário criar token separado.

## Publicar

1. Envie o código para `main`.
2. Crie e envie uma tag, por exemplo `v0.3.0`.
3. O workflow **MasterRGB Windows Build** compila, gera o ZIP portátil, o instalador e `SHA256SUMS.txt`, e anexa os três arquivos à Release da tag.

Também é possível executar o workflow manualmente em Actions → Run workflow. Nesse caso os arquivos ficam como artefatos da execução, sem criar Release.

## Enquanto o repositório for privado

A Release e os arquivos só podem ser baixados por quem tem acesso ao repositório. Para distribuir publicamente, torne o repositório público ou crie um repositório público separado e ajuste o passo de publicação do workflow.
