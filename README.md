# kicad-automation
### Command to compile & run

`$ gcc -ggdb <filename.c> -lm && ./a.out <arguments>`

for example:

`$ gcc -ggdb reorder-pins-1.c && ./a.out MT6315LP_B.kicad_sym`

---

The `-ggdb` flag let's you debug with `gdb`. If you don't need it you can skip it.

Ff you use Nvim you can slap this into your `init.lua` or other config. I will let you compile and run with `<leader>cr` from normal mode. It let's you specify arguments.
```lua
vim.keymap.set('n', '<leader>cr', function()
vim.cmd('w') -- save current file
    local file = vim.fn.expand('%')
    local out = vim.fn.expand('%<')
    -- prompt for command-line arguments
    local args = vim.fn.input('Arguments: ')
    -- compile and run with the provided arguments
    local cmd = string.format('split | term gcc -ggdb %s -o %s -lm && ./%s %s', file, out, out, args)
    vim.cmd(cmd)
end, { desc = 'Compile and run C file with arguments' })

```
