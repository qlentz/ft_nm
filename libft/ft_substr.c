/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 13:58:48 by qlentz            #+#    #+#             */
/*   Updated: 2022/12/01 19:04:16 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_substr(const char *s, unsigned int start, size_t len)
{
	char		*sub;
	size_t		slen;

	slen = ft_strlen(s);
	if (start >= slen)
	{
		sub = (char *)malloc(sizeof(char) * 1);
		sub[0] = 0;
		if (!sub)
			return (NULL);
		return (sub);
	}
	if (start + len > slen)
		len = slen - start;
	sub = (char *)malloc(sizeof(char) * (len + 1));
	if (!sub || !s)
		return (NULL);
	slen = 0;
	sub[len] = 0;
	while (*(s + slen + start) && len-- > 0)
	{
		sub[slen] = *(s + start + slen);
		slen++;
	}
	return (sub);
}
