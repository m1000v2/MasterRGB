# Matriz de compatibilidade MasterRGB

## Hardware testado no PC de referência

| Componente | Modelo | Detecção | Controle observado | Situação |
|---|---|---:|---|---|
| RAM | 4 × XPG Spectrix D60G DDR4 | Sim | Efeitos de RAM funcionam | Testado no PC de referência |
| GPU | PowerColor Red Devil RX 6700 XT (`1002:73DF`, `148C:2409`) | Sim | Efeitos predefinidos aplicam; mudança de cor não funciona corretamente | Parcial; precisa corrigir escrita de cor no Windows |
| Placa-mãe | MSI MPG B550 Gaming Plus / MS-7C56 | Sim | Zonas onboard reconhecidas | Testado para detecção |

## Hardware em avaliação, não certificado

| Componente | Modelo | Situação |
|---|---|---|
| GPU | EVGA GeForce RTX 3070 XC3 | Controlador EVGA existente no núcleo; ainda não testada fisicamente com MasterRGB |
| GPU | Gigabyte AORUS GeForce RTX 3060 Elite 12 GB | Revisão Rev. 1.0/2.0 não identificada; ainda não testada fisicamente |

## Regras de comunicação

- Não classificar uma placa como `Testada` apenas porque existe um detector no código.
- Registrar modelo, revisão, PCI IDs e resultados dos efeitos.
- Diferenciar `Detectada`, `Efeitos aplicados` e `Cores personalizadas validadas`.
- Não enviar transações I2C desconhecidas nem experimentar registradores não documentados em equipamento do usuário.
