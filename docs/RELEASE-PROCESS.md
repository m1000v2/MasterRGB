# Publicação e atualização do MasterRGB

## Estrutura recomendada do GitHub

- `main`: código revisado e pronto para releases.
- `preview`: integração e testes ainda não estáveis (opcional).
- GitHub Actions: build em `windows-2022`.
- GitHub Releases: pacote portable e instalador.

## Antes de publicar

1. Conferir licença GPL-2.0 e `NOTICE.md`.
2. Fazer build no workflow Windows.
3. Testar no PC de referência com XPG D60G, Red Devil e MSI B550.
4. Testar instalação e desinstalação numa VM Windows limpa.
5. Completar `docs/TEST-CHECKLIST-v0.2.3.md`.
6. Publicar como Pre-release até passar os testes críticos.

## Versões

- `v0.2.3-preview`: reparo visual e nova navegação inicial; manter como Preview até validação física.
- `v0.2.3`: somente após fechar pendências críticas e testes físicos.
- `v0.3.0`: próxima etapa de substituição mais ampla dos editores herdados por uma interface nativa MasterRGB.

## Atualizador

O Control Center consulta GitHub Releases quando o repositório é configurado em `config/support.json` / `%APPDATA%\\MasterRGB\\support.json`. A aplicação informa a versão e abre a página para revisão e download manual. Não instala atualização silenciosamente nem substitui arquivos em uso.


## Publicação com repositório de desenvolvimento privado

O código-fonte e o workflow ficam no repositório privado `m1000v2/MasterRGB`.
As versões públicas são publicadas em `m1000v2/MasterRGB-Releases`.

Antes de usar uma tag `v*`, crie o repositório público de Releases e configure no repositório privado o segredo de Actions `MASTER_RGB_RELEASE_TOKEN`. Use um fine-grained token limitado ao repositório `MasterRGB-Releases`, com permissão `Contents: Read and write`. Não inclua o token no código, no instalador ou no aplicativo.

O disparo manual (`workflow_dispatch`) disponibiliza os arquivos como artefatos da execução. O disparo por tag (por exemplo `v0.2.3-preview`) também publica os instaladores na página Releases do repositório público.
