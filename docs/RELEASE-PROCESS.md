# Processo de release do MasterRGB

1. Atualizar `CHANGELOG.md` e a versão em `installer/MasterRGB.iss`.
2. Rodar `scripts/Build-MasterRGB.ps1` em uma árvore limpa.
3. Compilar `installer/MasterRGB.iss` com Inno Setup 6.
4. Testar o checklist em `docs/TEST-CHECKLIST-v0.3.0.md`.
5. Conferir `dist/SHA256SUMS.txt`.
6. Enviar para `main` e criar a tag (`v0.3.0` ou `v0.3.1-preview`).
7. O workflow compila de novo e anexa ZIP, instalador e checksums à Release da tag.

## Artefatos

- `MasterRGB-0.3.0-Portable.zip`
- `MasterRGB-Setup-0.3.0.exe`
- `SHA256SUMS.txt`

## Regra de versão

- `vX.Y.Z-preview`: uso interno e validação.
- `vX.Y.Z`: somente após testes em máquina limpa e no hardware de referência.
