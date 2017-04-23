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

 .section ".rodata"
 .globl static_template_page
 .type static_template_page, STT_OBJECT
static_template_page:
 .incbin "template_page.html"
 .byte 0
 .size static_template_page, .-static_template_page

 .section ".rodata"
 .globl static_page_connect
 .type static_page_connect, STT_OBJECT
static_page_connect:
 .incbin "page_connect.html"
 .byte 0
 .size static_page_connect, .-static_page_connect

 .section ".rodata"
 .globl static_page_manage
 .type static_page_manage, STT_OBJECT
static_page_manage:
 .incbin "page_manage.html"
 .byte 0
 .size static_page_manage, .-static_page_manage

  .section ".rodata"
 .globl static_page_tcl
 .type static_page_tcl, STT_OBJECT
static_page_tcl:
 .incbin "page_tcl.html"
 .byte 0
 .size static_page_tcl, .-static_page_tcl

