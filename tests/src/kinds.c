/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kinds.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:45:29 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 23:35:09 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Weak, undefined, TLS, local and constant symbols. */
#pragma weak g_weak_undef_var
#pragma weak g_weak_def_var
#pragma weak weak_undef_func
#pragma weak weak_def_func

extern int			g_ext_var;
extern __thread int	g_tls_data;
extern __thread int	g_tls_bss;
extern int			g_weak_undef_var;
int					g_weak_def_var = 3;
static const char	g_local_ro[] = "ro";
static int			g_local_data = 5;

int	ext_func(void);
int	weak_undef_func(void);

int	weak_def_func(void)
{
	return (1);
}

static int	local_func(void)
{
	return (g_local_data + g_local_ro[0]);
}

int	use_all(void)
{
	return (g_ext_var + ext_func() + local_func() + g_tls_data + g_tls_bss
		+ (&g_weak_undef_var != 0) + (weak_undef_func != 0) + g_weak_def_var);
}
