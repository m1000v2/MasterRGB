# MasterRGB 0.3.0 — notas de correção

### O que mudou

1. **Páginas nativas:** a interface MasterRGB deixou de ser um bloco injetado com dezenas de substituições de texto. Início, Apoiar e o tema agora vivem em `qt/MasterRGBUi.cpp` e `qt/MasterRGBTheme.cpp`.
2. **Início:** a antiga “Central de iluminação” foi renomeada e reescrita. Explica o que é, lista os dispositivos detectados e oferece atalhos para Dispositivos, Configurações e Atualizações.
3. **Apoiar:** não há mais campos vazios com “https://paypal.me/seu-link”. O GitHub configurado aparece como botão e link; o PayPal só aparece quando existe um endereço real em `config/support.json`.
4. **Sobre:** versão real do produto, Qt, Windows e link do projeto. Os valores são reaplicados depois de `retranslateUi`, então não voltam ao texto de exemplo ao trocar o idioma.
5. **Informações:** ao clicar na aba superior, a subnavegação abre em “Sobre o MasterRGB”, em vez de mostrar o último dispositivo visitado.
6. **Tema:** a subaba Aparência foi removida. O seletor existente em Configurações > Geral aplica Claro/Escuro/Automático na hora, mudando paleta e QSS, e grava `%APPDATA%\MasterRGB\theme.txt`.
7. **Instalador:** Inno Setup nativo, em português, sem PowerShell, sem console e sem tela de DOS. Atalhos apontam para `MasterRGB.exe`.
8. **Iniciar com o Windows:** atalho em `{userstartup}\MasterRGB.lnk`, visível e desativável em Gerenciador de Tarefas > Aplicativos de inicialização. Nada de tarefa agendada.
9. **Último perfil:** ao iniciar minimizado, o aplicativo lê `%APPDATA%\OpenRGB\MasterRGB_LastProfile.txt` e carrega o perfil quando a lista estiver pronta.

### O que testar

- Instalar a 0.3.0 por cima da 0.2.3 e abrir pelo atalho.
- Em Configurações > Geral, alternar Claro/Escuro e confirmar que muda sem reiniciar.
- Em Apoiar, confirmar que não há campos para preencher e que o GitHub abre.
- Ir a Dispositivos, abrir um dispositivo, clicar em Informações e confirmar que abre em “Sobre o MasterRGB”.
- Ativar “Iniciar com o Windows” no instalador ou em Configurações > Geral, reiniciar e conferir o item no Gerenciador de Tarefas.
- Confirmar que o seletor HSV e os controles de dispositivo continuam funcionando.
