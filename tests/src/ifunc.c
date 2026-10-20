/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ifunc.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:45:29 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 23:35:09 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* GNU indirect functions (nm type 'i'). */
static int	impl(void)
{
	return (1);
}

static void	*resolve(void)
{
	return (impl);
}

int			global_ifunc(void) __attribute__((ifunc("resolve")));
static int	local_ifunc(void) __attribute__((ifunc("resolve")));

int	use(void)
{
	return (local_ifunc());
}
