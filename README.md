# Custom Shell
This is a custom shell that I made so I could learn about the `fork` syscall as well as processes and how to manage them using C. This shell is far from complete, since it is merely a project to learn these concepts.

## Customization
Customizing this shell is actually quite simple. Just navigate to `~/.config/myshellconfig.toml` and add the following.
```toml
[colours]
    prompt = "____"
```
Currently the only colours that are supported are
- Red / `red`
- Green / `green`