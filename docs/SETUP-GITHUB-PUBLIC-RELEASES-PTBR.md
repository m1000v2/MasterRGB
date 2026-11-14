# Configurar publicação pública do MasterRGB

O código-fonte e o workflow de compilação ficam no repositório privado `m1000v2/MasterRGB`. Os arquivos destinados aos usuários finais são publicados no repositório público `m1000v2/MasterRGB-Releases`.

## Preparação única

1. Criar `MasterRGB-Releases` como repositório **público** na conta `m1000v2`. Pode iniciar com um README.
2. Criar um token Fine-grained em GitHub → Settings → Developer settings → Personal access tokens → Fine-grained tokens.
3. Em Resource owner, selecionar `m1000v2`; em Repository access, selecionar somente `MasterRGB-Releases`.
4. Conceder `Contents: Read and write`.
5. No repositório privado `MasterRGB`, abrir Settings → Secrets and variables → Actions → New repository secret. Nome: `MASTER_RGB_RELEASE_TOKEN`. Valor: o token criado.

Não colocar o token no código-fonte, em arquivos `.ps1`, no instalador ou no aplicativo. O workflow o usa apenas durante a publicação.

## Como publicar uma versão de teste

- Executar Actions → MasterRGB Windows Build → Run workflow para compilar e anexar os arquivos como artefatos da execução privada.
- Para criar uma Release pública, enviar uma tag `v*`, por exemplo `v0.2.3-preview`. O workflow compila novamente e publica o ZIP portátil, o instalador e `SHA256SUMS.txt` no repositório público.

A publicação pública depende de o repositório `MasterRGB-Releases` existir e do segredo `MASTER_RGB_RELEASE_TOKEN` estar configurado. Sem o segredo, a compilação e os artefatos da execução continuam disponíveis, mas a Release pública é ignorada.
