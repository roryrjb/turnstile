# turnstile

> Markdown FastCGI server, uses [discount](https://www.pell.portland.or.us/~orc/Code/markdown/)

### Usage

__Example usage using Nginx:__

```bash
$ turnstile --host localhost --port 9123
```

`nginx.conf`:

```nginx
http {
    server {

        # ...

        location ~ \.md$ {
            include fastcgi_params;
            fastcgi_pass localhost:9123;
        }
    }
}
```
