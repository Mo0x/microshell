#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void err(char *s)
{
	while (*s)
		write(2, s++, 1);
}

int	cd(char **argv, int i)
{
	if (i !=2)
		return err("too many args\n"), 1;
	if (chdir(argv[1]) == -1)
		return err("cannot change directory to "), err(argv[1]), err("\n"), 1;
	return 0;
}

void set_pipe(int has_pipe, int *fd, int end)
{
	if (has_pipe && (dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		err("error: fatal\n"), exit(1);
}

int exec(char **argv, int i, char **envp)
{
	int has_pipe, fd[2], pid, status;

	has_pipe = argv[i] && !strcmp(argv[i], "|");
	if (!has_pipe && !strcmp(*argv, "cd"))
		return cd(argv, i);
	if (has_pipe && pipe(fd) == -1)
		err("error: fatal\n"), exit(1);
	if ((pid = fork()) == -1)
		err("error: fatal\n"), exit(1);
	if (!pid)
	{
		argv[i] = 0;
		set_pipe(has_pipe, fd, 1);
		if (!strcmp(*argv, "cd"))
			exit(cd(argv, i));
		execve(*argv, argv, envp);
		err("error: cannot execute "), err(*argv), err("\n"), exit(1);
	}
	waitpid(pid, &status, 0);
	set_pipe(has_pipe, fd, 0);
	return WIFEXITED(status) && WEXITSTATUS(status);

}

int main(int ac, char ** argv, char ** envp)
{
	(void)ac;
	int i= 0, status = 0;
	while (argv[i])
	{
		argv += i + 1;
		i = 0;
		if (strcmp(argv[i], "|") && strcmp(argv[i], ";"))
			i++;
		if (i)
			status = exec(argv, i, envp);
	}
	return status;
}

