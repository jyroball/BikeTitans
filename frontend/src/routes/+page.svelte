<script lang="ts">
	import { onMount } from 'svelte';
	import { base } from '$app/paths';
	import type { LocationsMap, TableRow } from '$lib/types';

	let tableData: TableRow[] = [];
	let sortField: 'location' | 'open_slots' | 'total_slot_number' = 'location';
	let sortDirection: 'asc' | 'desc' = 'asc';
	let isLoading: boolean = true;
	let error: string | null = null;

	onMount(async () => {
		try {
			// Use the base path for fetching data
			const response = await fetch(`${base}/output.json`);
			if (!response.ok) {
				throw new Error(`HTTP error! Status: ${response.status}`);
			}

			const locationsMap: LocationsMap = await response.json();

			// Convert the object structure to array format for table display
			tableData = Object.entries(locationsMap).map(([location, data]) => ({
				location,
				...data
			}));

			sortTable(sortField, sortDirection);
			isLoading = false;
		} catch (err) {
			console.error('Failed to load data:', err);
			error = err instanceof Error ? err.message : 'Unknown error occurred';
			isLoading = false;
		}
	});

	function sortTable(
		field: 'location' | 'open_slots' | 'total_slot_number',
		direction: 'asc' | 'desc'
	): void {
		sortField = field;
		sortDirection = direction;

		tableData = [...tableData].sort((a, b) => {
			let comparison = 0;

			if (field === 'location') {
				comparison = a[field].localeCompare(b[field]);
			} else {
				comparison = a[field] - b[field];
			}

			return direction === 'asc' ? comparison : -comparison;
		});
	}

	function toggleSort(field: 'location' | 'open_slots' | 'total_slot_number'): void {
		const direction = field === sortField && sortDirection === 'asc' ? 'desc' : 'asc';
		sortTable(field, direction);
	}

	function capitalizeFirstLetter(string: string): string {
		return string.charAt(0).toUpperCase() + string.slice(1);
	}
</script>

// src/routes/+page.svelte
<svelte:head>
	<title>Home | YourCompany</title>
</svelte:head>

<section class="mb-12">
	<div class="mb-16 text-center">
		<h1 class="mb-4 text-4xl font-bold">Welcome to YourCompany</h1>
		<p class="mx-auto max-w-2xl text-xl text-gray-600">
			We provide innovative solutions for your needs. Check our availability below.
		</p>
	</div>

	<div class="mb-16 grid grid-cols-1 gap-8 md:grid-cols-2">
		<div class="rounded-lg bg-blue-50 p-8 shadow-md">
			<h2 class="mb-4 text-2xl font-bold">Our Product</h2>
			<p class="mb-4">
				Our product helps you monitor and manage occupancy data across multiple locations. See
				real-time availability and make informed decisions with our intuitive interface.
			</p>
			<img
				src="{base}/images/product-image.png"
				alt="Product"
				class="h-64 w-full rounded-lg object-cover shadow-md"
			/>
		</div>

		<div class="rounded-lg bg-green-50 p-8 shadow-md">
			<h2 class="mb-4 text-2xl font-bold">Why Choose Us</h2>
			<ul class="list-disc space-y-2 pl-5">
				<li>Real-time occupancy tracking</li>
				<li>Comprehensive location analytics</li>
				<li>User-friendly dashboard</li>
				<li>Customizable alert system</li>
			</ul>
			<div class="mt-6">
				<a
					href="{base}/about"
					class="rounded-md bg-green-600 px-6 py-2 text-white transition-colors hover:bg-green-700"
				>
					Learn More
				</a>
			</div>
		</div>
	</div>

	<div class="overflow-hidden rounded-lg bg-white shadow-lg">
		<div class="border-b bg-gray-50 p-6">
			<h2 class="text-2xl font-bold">Available Locations</h2>
			<p class="text-gray-600">Click on column headers to sort the table</p>
		</div>

		{#if isLoading}
			<div class="p-8 text-center text-gray-500">Loading data...</div>
		{:else if error}
			<div class="p-8 text-center text-red-500">
				Error loading data: {error}
			</div>
		{:else if tableData.length > 0}
			<div class="overflow-x-auto">
				<table class="w-full">
					<thead class="bg-gray-100">
						<tr>
							<th
								class="cursor-pointer px-6 py-3 text-left text-sm font-medium tracking-wider text-gray-600 uppercase hover:bg-gray-200"
								on:click={() => toggleSort('location')}
							>
								Location
								{#if sortField === 'location'}
									<span>{sortDirection === 'asc' ? '↑' : '↓'}</span>
								{/if}
							</th>
							<th
								class="cursor-pointer px-6 py-3 text-left text-sm font-medium tracking-wider text-gray-600 uppercase hover:bg-gray-200"
								on:click={() => toggleSort('open_slots')}
							>
								Open Slots
								{#if sortField === 'open_slots'}
									<span>{sortDirection === 'asc' ? '↑' : '↓'}</span>
								{/if}
							</th>
							<th
								class="cursor-pointer px-6 py-3 text-left text-sm font-medium tracking-wider text-gray-600 uppercase hover:bg-gray-200"
								on:click={() => toggleSort('total_slot_number')}
							>
								Total Slots
								{#if sortField === 'total_slot_number'}
									<span>{sortDirection === 'asc' ? '↑' : '↓'}</span>
								{/if}
							</th>
						</tr>
					</thead>
					<tbody class="divide-y divide-gray-200 bg-white">
						{#each tableData as item}
							<tr class="hover:bg-gray-50">
								<td class="px-6 py-4 text-sm font-medium whitespace-nowrap text-gray-900">
									{capitalizeFirstLetter(item.location)}
								</td>
								<td class="px-6 py-4 text-sm whitespace-nowrap text-gray-500">
									{item.open_slots}
								</td>
								<td class="px-6 py-4 text-sm whitespace-nowrap text-gray-500">
									{item.total_slot_number}
								</td>
							</tr>
						{/each}
					</tbody>
				</table>
			</div>
		{:else}
			<div class="p-8 text-center text-gray-500">No data available.</div>
		{/if}
	</div>
</section>
