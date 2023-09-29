/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 12:06:28 by qlentz            #+#    #+#             */
/*   Updated: 2022/10/10 12:06:29 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dest, const char *src, size_t size)
{
	size_t	i;
	size_t	len;

	i = 0;
	len = ft_strlen(dest);
	if (size == 0)
		return (ft_strlen(src));
	while ((i + len) < (size - 1) && src[i])
	{
		dest[len + i] = src[i];
		i++;
	}
	dest[len + i] = 0;
	if (size > len)
		return (len + ft_strlen(src));
	return (ft_strlen(src) + size);
}
