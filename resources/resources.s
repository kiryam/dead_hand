 .section ".rodata"
 .globl static_style
 .type static_style, STT_OBJECT
static_style:
 .incbin "style.css"
 .byte 0
 .size static_style, .-static_style


 .section ".rodata"
 .globl static_javascript
 .type static_javascript, STT_OBJECT
static_javascript:
 .incbin "script.js"
 .byte 0
 .size static_javascript, .-static_javascript
