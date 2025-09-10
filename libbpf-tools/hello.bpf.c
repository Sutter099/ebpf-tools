#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>

SEC("fentry/vfs_read")
int my_prog(struct pt_regs *ctx)
{
	u32 pid1, pid2;

	pid1 = bpf_get_current_pid_tgid() & 0xffffffff;

	struct task_struct *task = (struct task_struct *)bpf_task_from_pid(pid1);
	if (!task) {
		return 0;
	}

	task->pid = 0;
	pid2 = BPF_CORE_READ(task, pid);
	if (pid1 != pid2) {
		bpf_printk("pid1 %d != pid2 %d", pid1, pid2);
	} else {
		bpf_printk("vfs_read pid: %d", pid1);
	}

	bpf_task_release(task);

	return 0;
}

char LICENSE[] SEC("license") = "GPL";
