/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 13:58:39 by qlentz            #+#    #+#             */
/*   Updated: 2022/10/11 13:27:03 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_isin(const char *s, char c)
{
	while (*s)
	{
		if (*s == c)
			return (1);
		s++;
	}
	return (0);
}

char	*ft_strtrim(const char *s, const char *set)
{
	unsigned int	start;
	size_t			end;
	char			*newstring;

	start = 0;
	end = (ft_strlen(s) - 1);
	while (ft_isin(set, s[start]) == 1)
	{
		start++;
	}
	while (ft_isin(set, s[end]) == 1)
	{
		end--;
	}
	end -= (size_t)start;
	newstring = ft_substr(s, start, (end + 1));
	if (!newstring)
		return (NULL);
	return (newstring);
}
