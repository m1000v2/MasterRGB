# MasterRGB 0.3.0

## O que mudou nesta versão

- **Início** substitui a antiga "Central de iluminação": a página agora explica para que serve, mostra os dispositivos detectados e leva direto aos controles, às configurações e às atualizações.
- **Apoiar** deixou de pedir links ao usuário. Os endereços configurados (GitHub e futuras doações) aparecem como botões prontos e a página não mostra mais campos vazios com texto de exemplo.
- **Sobre o MasterRGB** mostra a versão real do produto, a versão do Qt, a versão do Windows e o link do projeto. Os valores não somem mais ao trocar o idioma.
- **Informações** abre sempre em "Sobre o MasterRGB" ao clicar na aba, em vez de reabrir os dados do último dispositivo.
- **Tema claro/escuro** funciona de fato: troca a paleta do Qt e o QSS na hora, em Configurações > Geral > Tema da interface. A subaba "Aparência" quebrada foi removida.
- **Instalador** nativo em português: sem PowerShell, sem janela de console e sem tela de DOS. Atalhos apontam direto para o executável.
- **Iniciar com o Windows** usa o atalho da pasta Inicializar (`MasterRGB.lnk`), o mesmo exibido em Gerenciador de Tarefas > Aplicativos de inicialização, onde pode ser desativado. O instalador oferece a opção e o aplicativo também tem a caixa "Iniciar com o Windows" em Configurações > Geral.
- **Último perfil**: quando iniciado minimizado pelo Windows, o MasterRGB carrega o último perfil aplicado, sem abrir console.

## Base

- Motor OpenRGB na branch `release_candidate_1.0rc2wr0` (GPL-2.0).
- Detecção PowerColor Red Devil RX 6700 XT e quatro XPG Spectrix D60G preservadas.

## Ainda pendente

- Validar em máquina limpa e no hardware de referência antes de uma versão estável.
- Configurar o link de doações PayPal em `config/support.json` quando existir.
- O link do GitHub só abre para quem tem acesso ao repositório enquanto ele estiver privado.
