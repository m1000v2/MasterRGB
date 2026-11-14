# MasterRGB 0.3.0 — Checklist de validação

## Instalação

- [ ] Instalar por cima da 0.2.3; o instalador abre em português e sem console.
- [ ] O atalho do Menu Iniciar abre `MasterRGB.exe` diretamente.
- [ ] Marcar "Iniciar o MasterRGB junto com o Windows" e confirmar o atalho na pasta Inicializar.
- [ ] O item aparece em Gerenciador de Tarefas > Aplicativos de inicialização e pode ser desativado.
- [ ] Desinstalar remove o atalho de inicialização.

## Interface

- [ ] Aba Início explica o painel e lista os dispositivos detectados.
- [ ] "Abrir controles" leva ao dispositivo correto.
- [ ] Apoiar não mostra campos vazios; o GitHub abre no navegador.
- [ ] Sobre mostra MasterRGB 0.3.0, versão do Qt, versão do Windows e o link do projeto.
- [ ] Trocar o idioma não volta os valores para "Qt Version Value"/"OS Version Value".
- [ ] Depois de abrir um dispositivo em Informações, clicar na aba superior abre "Sobre o MasterRGB".

## Tema

- [ ] Configurações > Geral > Tema da interface alterna Claro/Escuro na hora.
- [ ] Reiniciar mantém a escolha.
- [ ] O seletor HSV, os ícones e as páginas de dispositivo continuam legíveis nos dois temas.

## Inicialização e perfil

- [ ] Iniciar pelo atalho de inicialização abre minimizado.
- [ ] O último perfil aplicado é carregado (`%APPDATA%\OpenRGB\MasterRGB_LastProfile.txt`).
- [ ] Nenhuma janela de PowerShell ou console aparece.

## Regressão de hardware

- [ ] Quatro XPG Spectrix D60G continuam detectadas.
- [ ] PowerColor Red Devil RX 6700 XT continua registrada.
- [ ] Efeitos e perfil estático continuam sendo aplicados.
