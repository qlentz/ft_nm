/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 13:57:59 by qlentz            #+#    #+#             */
/*   Updated: 2022/10/10 13:58:01 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *s1, const char *s2, size_t n)
{
	size_t	i;
	size_t	j;
	size_t	s2len;

	i = 0;
	j = 0;
	s2len = ft_strlen(s2);
	if (s2len == 0)
		return ((char *)s1);
	while (s1[i] && i < n)
	{
		j = 0;
		while (s1[i + j] && s2[j] && s1[i + j] == s2[j] && i + j < n)
			j++;
		if (j == s2len)
			return ((char *)(s1 + i));
		i++;
	}
	return (NULL);
}
